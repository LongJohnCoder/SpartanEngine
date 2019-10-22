/*
Copyright(c) 2016-2019 Panos Karabelas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

//= INCLUDES ==============
#include "FileSystem.h"
#include <filesystem>
#include <regex>
#include <fstream>
#include <sstream> 
#include "../Logging/Log.h"
#include <Windows.h>
#include <shellapi.h>
//=========================

//= NAMESPACES =====
using namespace std;
//==================

namespace Spartan
{
	vector<string> FileSystem::m_supportedImageFormats;
	vector<string> FileSystem::m_supportedAudioFormats;
	vector<string> FileSystem::m_supportedModelFormats;
	vector<string> FileSystem::m_supportedShaderFormats;
	vector<string> FileSystem::m_supportedScriptFormats;
	vector<string> FileSystem::m_supportedFontFormats;

	void FileSystem::Initialize()
	{
		// Supported image formats
		m_supportedImageFormats =
		{
			".jpg",
			".png",
			".bmp",
			".tga",
			".dds",
			".exr",
			".raw",
			".gif",
			".hdr",
			".ico",
			".iff",
			".jng",
			".jpeg",
			".koala",
			".kodak",
			".mng",
			".pcx",
			".pbm",
			".pgm",
			".ppm",
			".pfm",
			".pict",
			".psd",
			".raw",
			".sgi",
			".targa",
			".tiff",
			".tif", // tiff can also be tif
			".wbmp",
			".webp",
			".xbm",
			".xpm"
		};

		// Supported audio formats
		m_supportedAudioFormats =
		{
			".aiff",
			".asf",
			".asx",
			".dls",
			".flac",
			".fsb",
			".it",
			".m3u",
			".midi",
			".mod",
			".mp2",
			".mp3",
			".ogg",
			".pls",
			".s3m",
			".vag", // PS2/PSP
			".wav",
			".wax",
			".wma",
			".xm",
			".xma" // XBOX 360
		};

		// Supported model formats
		m_supportedModelFormats =
		{
			".3ds",
			".obj",
			".fbx",
			".blend",
			".dae",
            ".gltf",
			".lwo",
			".c4d",
			".ase",
			".dxf",
			".hmp",
			".md2",
			".md3",
			".md5",
			".mdc",
			".mdl",
			".nff",
			".ply",
			".stl",
			".x",
			".smd",
			".lxo",
			".lws",
			".ter",
			".ac3d",
			".ms3d",
			".cob",
			".q3bsp",
			".xgl",
			".csm",
			".bvh",
			".b3d",
			".ndo"
		};

		// Supported shader formats
		m_supportedShaderFormats =
		{
			".hlsl"
		};

		// Supported script formats
		m_supportedScriptFormats = 
		{
			".as"
		};

		// Supported font formats
		m_supportedFontFormats =
		{
			".ttf",
			".ttc",
			".cff",
			".woff",
			".otf",
			".otc",
			".pfa",
			".pfb",
			".fnt",
			".bdf",
			".pfr"
		};
	}

    bool FileSystem::IsEmptyOrWhitespace(const std::string& var)
    {
        // Check if it's empty
        if (var.empty())
            return true;

        // Check if it's made out of whitespace characters
        for (char _char : var)
        {
            if (!std::isspace(_char))
                return false;
        }

        return true;
    }

    bool FileSystem::IsAlphanumeric(const std::string& var)
    {
        if (IsEmptyOrWhitespace(var))
            return false;

        for (char _char : var)
        {
            if (!std::isalnum(_char))
                return false;
        }

        return true;
    }

    string FileSystem::GetStringBeforeExpression(const string& str, const string& exp)
    {
        // ("The quick brown fox", "brown") -> "The quick "
        size_t position = str.find(exp);
        return position != string::npos ? str.substr(0, position) : "";
    }

    string FileSystem::GetStringAfterExpression(const string& str, const string& exp)
    {
        // ("The quick brown fox", "brown") -> "fox"
        size_t position = str.find(exp);
        return position != string::npos ? str.substr(position + exp.length()) : "";
    }

    string FileSystem::GetStringBetweenExpressions(const string& str, const string& exp_a, const string& exp_b)
    {
        // ("The quick brown fox", "The ", " brown") -> "quick"

        regex base_regex(exp_a + "(.*)" + exp_b);

        smatch base_match;
        if (regex_search(str, base_match, base_regex))
        {
            // The first sub_match is the whole string; the next
            // sub_match is the first parenthesized expression.
            if (base_match.size() == 2)
            {
                return base_match[1].str();
            }
        }

        return str;
    }

    string FileSystem::ConvertToUppercase(const string& lower)
    {
        locale loc;
        string upper;
        for (const auto& character : lower)
        {
            upper += std::toupper(character, loc);
        }

        return upper;
    }

    string FileSystem::ReplaceExpression(const string& str, const string& from, const string& to)
    {
        return regex_replace(str, regex(from), to);
    }

    wstring FileSystem::StringToWstring(const string& str)
    {
        const auto slength = static_cast<int>(str.length()) + 1;
        const auto len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, nullptr, 0);
        const auto buf = new wchar_t[len];
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
        std::wstring result(buf);
        delete[] buf;
        return result;
    }

    vector<string> FileSystem::GetIncludedFiles(const std::string& file_path)
    {
        // Read the file
        ifstream in(file_path);
        stringstream buffer;
        buffer << in.rdbuf();

        string source = buffer.str();
        string directory = GetDirectoryFromFilePath(file_path);
        string directive_exp = "#include \"";
        vector<string> file_paths;

        // Early exit if there is no include directive
        if (source.find(directive_exp) == string::npos)
            return file_paths;

        // Scan for include directives
        istringstream stream(source);
        string include_directive;
        while (std::getline(stream, include_directive))
        {
            if (include_directive.find(directive_exp) != string::npos)
            {
                // Construct file path and save it
                string file_name = GetStringBetweenExpressions(include_directive, directive_exp, "\"");
                file_paths.emplace_back(directory + file_name);
            }
        }

        // If any file path contains more file paths inside, start resolving them recursively
        auto file_paths_copy = file_paths; // copy the file paths to avoid modification while iterating
        for (const auto& _file_path : file_paths_copy)
        {
            // Read the file
            ifstream _in(_file_path);
            stringstream _buffer;
            _buffer << _in.rdbuf();

            // Check for include directive
            string source = _buffer.str();
            if (source.find(directive_exp) != string::npos)
            {
                auto new_includes = GetIncludedFiles(_file_path);
                file_paths.insert(file_paths.end(), new_includes.begin(), new_includes.end());
            }
        }

        // At this point, everything should be resolved
        return file_paths;
    }

    void FileSystem::OpenDirectoryWindow(const string& directory)
    {
        ShellExecute(nullptr, nullptr, StringToWstring(directory).c_str(), nullptr, nullptr, SW_SHOW);
    }

    bool FileSystem::CreateDirectory_(const string& path)
	{
		try
		{
			return filesystem::create_directories(path);
		}
		catch (filesystem::filesystem_error& e)
		{
			LOG_WARNING("%s, %s", e.what(), path.c_str());
			return true;
		}
	}

	bool FileSystem::DeleteDirectory(const string& path)
	{
		try
		{
			return filesystem::remove_all(path);
		}
		catch (filesystem::filesystem_error& e)
		{
			LOG_WARNING("s, %s", e.what(), path.c_str());
			return true;
		}
	}

    bool FileSystem::Exists(const string& path)
	{
		try
		{
			return filesystem::exists(path);
		}
		catch (filesystem::filesystem_error& e)
		{
			LOG_WARNING("%s, %s", e.what(), path.c_str());
			return false;
		}
	}

    bool FileSystem::IsFile(const string& path)
    {
        try
        {
            return filesystem::is_regular_file(path);
        }
        catch (filesystem::filesystem_error & e)
        {
            LOG_WARNING("%s, %s", e.what(), path.c_str());
            return false;
        }
    }

    bool FileSystem::IsDirectory(const string& path)
    {
        try
        {
            return filesystem::is_directory(path);
        }
        catch (filesystem::filesystem_error & e)
        {
            LOG_WARNING("%s, %s", e.what(), path.c_str());
            return false;
        }
    }

	bool FileSystem::DeleteFile_(const string& path)
	{
		// If this is a directory path, return
		if (filesystem::is_directory(path))
			return false;

		try
		{
			return remove(path.c_str()) == 0;
		}
		catch (filesystem::filesystem_error& e)
		{
			LOG_WARNING("%s, %s", e.what(), path.c_str());
			return true;
		}
	}

	bool FileSystem::CopyFileFromTo(const string& source, const string& destination)
	{
		if (source == destination)
			return true;

		// In case the destination path doesn't exist, create it
		if (!Exists(GetDirectoryFromFilePath(destination)))
		{
			CreateDirectory_(GetDirectoryFromFilePath(destination));
		}

		try 
		{
			return filesystem::copy_file(source, destination, filesystem::copy_options::overwrite_existing);
		}
		catch (filesystem::filesystem_error& e)
		{
			LOG_WARNING("%s", e.what());
			return true;
		}
	}

    string FileSystem::GetFileNameFromFilePath(const string& path)
	{
        filesystem::path fs_path = path;
        if (!fs_path.has_filename())
        {
            LOG_WARNING("\"%s\" has no file name", path.c_str());
            return "";
        }

        return fs_path.filename().generic_string();
	}

	string FileSystem::GetFileNameNoExtensionFromFilePath(const string& path)
	{
		auto file_name		= GetFileNameFromFilePath(path);
        size_t last_index	= file_name.find_last_of('.');

        if (last_index != string::npos)
		    return file_name.substr(0, last_index);

        LOG_WARNING("Failed to extract file name from \"%s\"", path.c_str());
		return "";
	}

    string FileSystem::GetDirectoryFromFilePath(const string& path)
	{
        size_t last_index = path.find_last_of("\\/");

        if (last_index != string::npos)
		    return path.substr(0, last_index + 1);

        LOG_WARNING("Failed to extract directory from \"%s\"", path.c_str());
		return "";
	}

	string FileSystem::GetFilePathWithoutExtension(const string& path)
	{
		auto directory		= GetDirectoryFromFilePath(path);
		auto fileNameNoExt	= GetFileNameNoExtensionFromFilePath(path);

		return directory + fileNameNoExt;
	}

	string FileSystem::GetExtensionFromFilePath(const string& path)
	{
        filesystem::path fs_path = path;
        if (!fs_path.has_extension())
        {
            LOG_WARNING("\"%s\" has no extension", path.c_str());
            return "";   
        }

        return fs_path.extension().generic_string();
	}

    string FileSystem::NativizeFilePath(const string& path)
    {
        string file_path_no_ext = GetFilePathWithoutExtension(path);

        if (IsSupportedAudioFile(path))    return file_path_no_ext + EXTENSION_AUDIO;
        if (IsSupportedImageFile(path))    return file_path_no_ext + EXTENSION_TEXTURE;
        if (IsSupportedModelFile(path))    return file_path_no_ext + EXTENSION_MODEL;
        if (IsSupportedFontFile(path))     return file_path_no_ext + EXTENSION_FONT;
        if (IsSupportedShaderFile(path))   return file_path_no_ext + EXTENSION_SHADER;

        LOG_WARNING("Failed to nativize file path");
        return path;
    }

    vector<string> FileSystem::GetDirectoriesInDirectory(const string& path)
	{
		vector<string> directories;
        filesystem::directory_iterator it_end; // default construction yields past-the-end
		for (filesystem::directory_iterator it(path); it != it_end; ++it)
		{
			if (!filesystem::is_directory(it->status()))
				continue;

            string path;

            // A system_error is possible if the characters are
            // something that can't be converted, like Russian.
            try
            {
                path = it->path().string();
            }
            catch (system_error& e)
            {
                LOG_WARNING("Failed to read a directory path. %s", e.what());
            }

            if (!path.empty())
            {
                // Replace double backward slashes with one backward slash
                std::replace(path.begin(), path.end(), '\\', '/');

                // finally, save
                directories.emplace_back(path);
            }
		}


		return directories;
	}

	vector<string> FileSystem::GetFilesInDirectory(const string& path)
	{
		vector<string> file_paths;
        filesystem::directory_iterator it_end; // default construction yields past-the-end
		for (filesystem::directory_iterator it(path); it != it_end; ++it)
		{
			if (!filesystem::is_regular_file(it->status()))
				continue;

            try
            {
                // a crash is possible if the characters are
                // something that can't be converted, like Russian.
                file_paths.emplace_back(it->path().string());
            }
            catch (system_error& e)
            {
                LOG_WARNING("Failed to read a file path. %s", e.what());
            }
		}

		return file_paths;
	}

	bool FileSystem::IsSupportedAudioFile(const string& path)
	{
		string extension = GetExtensionFromFilePath(path);

		auto supportedFormats = GetSupportedAudioFormats();
		for (const auto& format : supportedFormats)
		{
			if (extension == format || extension == ConvertToUppercase(format))
				return true;
		}

		return false;
	}

	bool FileSystem::IsSupportedImageFile(const string& path)
	{
		string extension = GetExtensionFromFilePath(path);

		auto supportedFormats = GetSupportedImageFormats();
		for (const auto& format : supportedFormats)
		{
			if (extension == format || extension == ConvertToUppercase(format))
				return true;
		}
		
		if (GetExtensionFromFilePath(path) == EXTENSION_TEXTURE)
			return true;

		return false;
	}

	bool FileSystem::IsSupportedModelFile(const string& path)
	{
		string extension = GetExtensionFromFilePath(path);

		auto supportedFormats = GetSupportedModelFormats();
		for (const auto& format : supportedFormats)
		{
			if (extension == format || extension == ConvertToUppercase(format))
				return true;
		}

		return false;
	}

	bool FileSystem::IsSupportedShaderFile(const string& path)
	{
		string extension = GetExtensionFromFilePath(path);

		auto supportedFormats = GetSupportedShaderFormats();
		for (const auto& format : supportedFormats)
		{
			if (extension == format || extension == ConvertToUppercase(format))
				return true;
		}

		return false;
	}

	bool FileSystem::IsSupportedFontFile(const string& path)
	{
		string extension = GetExtensionFromFilePath(path);

		auto supportedFormats = GetSupportedFontFormats();
		for (const auto& format : supportedFormats)
		{
			if (extension == format || extension == ConvertToUppercase(format))
				return true;
		}

		return false;
	}

	bool FileSystem::IsEngineScriptFile(const string& path)
	{
		string extension = GetExtensionFromFilePath(path);

		auto supportedFormats = GetSupportedScriptFormats();
		for (const auto& format : supportedFormats)
		{
			if (extension == format || extension == ConvertToUppercase(format))
				return true;
		}

		return false;
	}

	bool FileSystem::IsEnginePrefabFile(const string& path)
	{
		return GetExtensionFromFilePath(path) == EXTENSION_PREFAB;
	}

	bool FileSystem::IsEngineModelFile(const string& path)
	{
		return GetExtensionFromFilePath(path) == EXTENSION_MODEL;
	}

	bool FileSystem::IsEngineMaterialFile(const string& path)
	{
		return GetExtensionFromFilePath(path) == EXTENSION_MATERIAL;
	}

	bool FileSystem::IsEngineMeshFile(const string& path)
	{
		return GetExtensionFromFilePath(path) == EXTENSION_MESH;
	}

	bool FileSystem::IsEngineSceneFile(const string& path)
	{
		return GetExtensionFromFilePath(path) == EXTENSION_WORLD;
	}

	bool FileSystem::IsEngineTextureFile(const string& path)
	{
		return GetExtensionFromFilePath(path) == EXTENSION_TEXTURE;
	}

    bool FileSystem::IsEngineAudioFile(const std::string& path)
    {
        return GetExtensionFromFilePath(path) == EXTENSION_AUDIO;
    }

    bool FileSystem::IsEngineShaderFile(const string& path)
	{
		return GetExtensionFromFilePath(path) == EXTENSION_SHADER;
	}

    bool FileSystem::IsEngineFile(const string& path)
    {
        return  IsEngineScriptFile(path)   ||
                IsEnginePrefabFile(path)   ||
                IsEngineModelFile(path)    ||
                IsEngineMaterialFile(path) ||
                IsEngineMeshFile(path)     ||
                IsEngineSceneFile(path)    ||
                IsEngineTextureFile(path)  ||
                IsEngineAudioFile(path)    ||
                IsEngineShaderFile(path);
    }

    vector<string> FileSystem::GetSupportedFilesInDirectory(const string& path)
    {
        vector<string> filesInDirectory = GetFilesInDirectory(path);
        vector<string> imagesInDirectory = GetSupportedImageFilesFromPaths(filesInDirectory); // get all the images
        vector<string> scriptsInDirectory = GetSupportedScriptFilesFromPaths(filesInDirectory); // get all the scripts
        vector<string> modelsInDirectory = GetSupportedModelFilesFromPaths(filesInDirectory); // get all the models
        vector<string> supportedFiles;

        // get supported images
        for (const auto& imageInDirectory : imagesInDirectory)
        {
            supportedFiles.emplace_back(imageInDirectory);
        }

        // get supported scripts
        for (const auto& scriptInDirectory : scriptsInDirectory)
        {
            supportedFiles.emplace_back(scriptInDirectory);
        }

        // get supported models
        for (const auto& modelInDirectory : modelsInDirectory)
        {
            supportedFiles.emplace_back(modelInDirectory);
        }

        return supportedFiles;
    }

    vector<string> FileSystem::GetSupportedImageFilesFromPaths(const vector<string>& paths)
    {
        vector<string> imageFiles;
        for (const auto& path : paths)
        {
            if (!IsSupportedImageFile(path))
                continue;

            imageFiles.emplace_back(path);
        }

        return imageFiles;
    }

    vector<string> FileSystem::GetSupportedAudioFilesFromPaths(const vector<string>& paths)
    {
        vector<string> audioFiles;
        for (const auto& path : paths)
        {
            if (!IsSupportedAudioFile(path))
                continue;

            audioFiles.emplace_back(path);
        }

        return audioFiles;
    }

    vector<string> FileSystem::GetSupportedScriptFilesFromPaths(const vector<string>& paths)
    {
        vector<string> scripts;
        for (const auto& path : paths)
        {
            if (!IsEngineScriptFile(path))
                continue;

            scripts.emplace_back(path);
        }

        return scripts;
    }

    vector<string> FileSystem::GetSupportedModelFilesFromPaths(const vector<string>& paths)
    {
        vector<string> images;
        for (const auto& path : paths)
        {
            if (!IsSupportedModelFile(path))
                continue;

            images.emplace_back(path);
        }

        return images;
    }

    vector<string> FileSystem::GetSupportedModelFilesInDirectory(const string& path)
    {
        return GetSupportedModelFilesFromPaths(GetFilesInDirectory(path));
    }

    vector<string> FileSystem::GetSupportedSceneFilesInDirectory(const string& path)
    {
        vector<string> sceneFiles;

        auto files = GetFilesInDirectory(path);
        for (const auto& file : files)
        {
            if (!IsEngineSceneFile(file))
                continue;

            sceneFiles.emplace_back(file);
        }

        return sceneFiles;
    }

	string FileSystem::GetRelativePath(const string& path)
	{
        if (filesystem::path(path).is_relative())
            return path;

		// create absolute paths
        filesystem::path p = filesystem::absolute(path);
        filesystem::path r = filesystem::absolute(GetWorkingDirectory());

		// if root paths are different, return absolute path
		if( p.root_path() != r.root_path())
		    return p.generic_string();

		// initialize relative path
        filesystem::path result;

		// find out where the two paths diverge
		filesystem::path::const_iterator itr_path = p.begin();
		filesystem::path::const_iterator itr_relative_to = r.begin();
		while( *itr_path == *itr_relative_to && itr_path != p.end() && itr_relative_to != r.end() ) 
		{
		    ++itr_path;
		    ++itr_relative_to;
		}

		// add "../" for each remaining token in relative_to
		if( itr_relative_to != r.end() ) 
		{
		    ++itr_relative_to;
		    while( itr_relative_to != r.end() ) 
			{
		        result /= "..";
		        ++itr_relative_to;
		    }
		}

		// add remaining path
		while( itr_path != p.end() ) 
		{
		    result /= *itr_path;
		    ++itr_path;
		}

		return result.generic_string();
	}

	string FileSystem::GetWorkingDirectory()
	{
		return filesystem::current_path().generic_string();
	}

	string FileSystem::GetParentDirectory(const string& path)
	{
        return filesystem::path(path).parent_path().generic_string();
	}

    string FileSystem::GetRootDirectory(const std::string& path)
    {
        return filesystem::path(path).root_directory().generic_string();
    }
}

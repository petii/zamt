
#include <chrono>
#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

#include <boost/program_options.hpp>
#include <boost/dll.hpp>

#include "common.h"
#include "shader_directories.h"
#include "visualizationapp.h"

constexpr auto epsilon = 0.0001;

auto parseShaderDirectory(fs::path directory)
{
  auto result = std::make_optional<ShaderDirectoryContents>(
      {{FileType::directory, directory.filename()}});
  std::set<std::string> remainingFiles = {to_string(FileType::vertexShader),
                                          to_string(FileType::fragmentShader),
                                          to_string(FileType::indicesList)};
  const auto generatorName = fs::path("generate_indices.py");
  std::optional<fs::directory_entry> generator = std::nullopt;
  for (auto& file : fs::directory_iterator(directory)) {
    auto filename = file.path().filename();
    if (filename == generatorName) {
      std::clog << "  Found generator script: " << file.path() << std::endl;
      generator = file;
      continue;
    }
    if (remainingFiles.count(filename) == 0) {
      continue;
    }
    std::clog << "  Found " << file << std::endl;
    remainingFiles.erase(filename);
    result->emplace(from_string(filename), file.path());
  }
  if (remainingFiles.count(to_string(FileType::indicesList)) && generator) {
    // remainingFiles.emplace(to_string(FileType::indicesList));
    auto originalPath = fs::current_path();
    fs::current_path(generator->path().parent_path());
    std::stringstream generatorCommandStream;
    generatorCommandStream << "python " << generator->path().filename().string() << " -s "
                           << VisualizationApp::RESULT_SIZE << " -c "
                           << VisualizationApp::HISTORY_SIZE;
    std::clog << "  Generating " << to_string(FileType::indicesList)
              << " with command: " << std::endl
              << "    " << generatorCommandStream.str() << std::endl;
    // ehh...
    std::system(generatorCommandStream.str().c_str());
    fs::current_path(originalPath);
    auto dirIt = fs::directory_iterator(directory);
    auto indicesFile = std::find_if(fs::begin(dirIt), fs::end(dirIt), [&](auto& entry) {
      return from_string(entry.path().filename().string()) == FileType::indicesList;
    });
    if (indicesFile != fs::end(dirIt)) {
      result->emplace(FileType::indicesList, indicesFile->path());
      remainingFiles.erase(to_string(FileType::indicesList));
    }
  }
  if (!remainingFiles.empty()) {
    std::cerr << "  Could not find all required files. Missing: " << std::endl;
    for (auto& f : remainingFiles) {
      std::cerr << "    " << f << std::endl;
    }
    result = std::nullopt;
  }
  return result;
}

int main(int argc, char** argv)
{
  try {
    namespace options = boost::program_options;
    bool monitor = false;
    double overlap = 0.5;
    options::options_description description("Command line arguments");
    description.add_options()
      ("help", "display this text")
      ("monitor", options::value<bool>(&monitor), "turn on audio monitoring")
      ("overlap", options::value<double>(&overlap), "audio overlap ratio ( in [0,1) )");

    options::variables_map variableMap;
    options::store(options::parse_command_line(argc, argv, description), variableMap);
    options::notify(variableMap);

    if (variableMap.count("help")) {
      std::cout << description << std::endl;
      return 0;
    }

    std::cout << "Enumerating shaders..." << std::endl;

    fs::path executableDirectory{boost::dll::program_location().parent_path().string()};
    auto shaderDir = executableDirectory.parent_path() / "share" / EXECUTABLE_NAME / "shaders";
    std::vector<ShaderDirectoryContents> shaders;
    for (auto& dir : fs::directory_iterator(shaderDir)) {
      if (!dir.is_directory()) {
        continue;
      }
      std::clog << "Directory: " << dir << std::endl;
      auto shader = parseShaderDirectory(dir);
      if (!shader) {
        continue;
      }
      shaders.emplace_back(std::move(shader.value()));
    }
    std::sort(shaders.begin(),shaders.end(), [](auto& lhs, auto& rhs) {
      return lhs[FileType::directory].string() < rhs[FileType::directory].string();
    });
    std::clog << "Found shaders: " << std::endl;
    for (auto& shader : shaders) {
      std::clog << "  " << shader[FileType::directory] << std::endl;
    }

    auto originalWorkingDirectory = fs::current_path();
    fs::current_path(executableDirectory.parent_path());
    VisualizationApp app(monitor, clamp(0.0, 1.0 - epsilon, overlap), std::move(shaders));
    app.run();
  }
  catch (std::exception& e) {
    std::cerr << "Exception caught in " << __FUNCTION__ << ": " << e.what() << std::endl;
    return -1;
  }
  return 0;
}

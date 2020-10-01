#include "MapLoader.h"

std::vector<std::string> MapLoader::Split(std::string line) const {
  // Create a vector to store the strings
  std::vector<std::string> splitStrings;
  // Create indices variables for the start indicy and end indicy of the split
  // string
  size_t prev{0}, pos{0};
  do {
    // find the first blank space after the previous indicy
    pos = line.find(' ', prev);
    // if the position is at the end of the string, then set it equal to the
    // length.
    if (pos == std::string::npos) pos = line.length();
    // Create a substring between each of the indices
    std::string split = line.substr(prev, pos - prev);
    // If its not blank space (multiple spaces), then the string is split
    if (!split.empty()) splitStrings.push_back(split);
    // set the new start indicy to the end indicy plus 1.
    prev = pos + 1;
  } while (pos < line.length() && prev < line.length());
  return splitStrings;
}

void MapLoader::PreprocessContinents(std::string line, DataType* dataType) {
  // The line is split into separate words
  std::vector<std::string> words = Split(line);
  if (words.size() > 0) {
    // if the line contains words, then add another continent to the counter
    continentsCount++;
  } else {
    // When there are no more continents to process, an array of continents
    // sizes can be allocated
    continentsSizes = new int[continentsCount];
    // For each of the elemnts in the array, set them to 0
    for (int i = 0; i < continentsCount; i++) continentsSizes[i] = 0;
    // Alocate the vector and reserve the the amount of spaces to the number of
    // continents
    continents = std::vector<Continent*>();
    continents.reserve(continentsCount);
    // End processing of datatype
    *dataType = DataType::None;
  }
}

void MapLoader::PreprocessTerritories(std::string line, DataType* dataType) {
  // The line is split into separate words
  std::vector<std::string> words = Split(line);
  if (words.size() > 0) {
    // Territoies must have at least 3 data points (index, continent, and name)
    // for the file to be valid.
    if (words.size() < 3) validityData->validData = false;
    // incriment the territories count
    territoriesCount++;
    try {
      // incriment the continents sizes to which this territory belongs
      int belongsTo = std::stoi(words[2]);
      continentsSizes[belongsTo - 1]++;

    } catch (std::invalid_argument) {
      // Error, failure to read an integer.
      std::cout << "failure reading integer" << std::endl;
      validityData->validData = false;
    }
  } else {
    // End preprocess of territories, the neighbors sizes may now be allocated.
    *dataType = DataType::None;
    neighborsSizes = new int[territoriesCount];
  }
}

void MapLoader::PreprocessBorders(std::string line, DataType* dataType) {
  // The line is split into separate words
  std::vector<std::string> words = Split(line);
  if (words.size() > 0) {
    // The line must contain at least 2 data points (otherwise its a floating
    // territory with no edges).
    if (words.size() < 2) {
      validityData->validData = false;
      std::cout << "Error, territory has no edges... invalid map file"
                << std::endl;
    }
    try {
      // The number of neighbors is equal to the number of remaining indices
      neighborsSizes[std::stoi(words[0]) - 1] = words.size() - 1;
    } catch (std::invalid_argument) {
      std::cout << "failure reading integer" << std::endl;
      validityData->validData = false;
    }
  } else
    *dataType = DataType::None;
}

void MapLoader::ProcessBorders(std::string line, DataType* dataType) {
  // The line is split into separate words
  std::vector<std::string> words = Split(line);
  if (words.size() > 0) {
    // Each territory MUST have at least 1 border
    if (words.size() < 2) {
      validityData->validData = false;
      std::cout << "Error, territory has no edges... invalid map file"
                << std::endl;
    }
    try {
      // Get a pointer to the specific territory.
      std::vector<Territory*>* terrs = generatedMap->GetTerritories();
      Territory* territory =
          (*generatedMap->GetTerritories())[std::stoi(words[0]) - 1];
      // set the neighbors with the corresponding indices
      for (int i = 0; i < words.size(); i++)
        territory->AddNeighbor(
            (*generatedMap->GetTerritories())[std::stoi(words[i]) - 1]);

    } catch (std::invalid_argument) {
      std::cout << "failure reading integer" << std::endl;
      validityData->validData = false;
    }
  } else
    *dataType = DataType::None;
}

void MapLoader::ProcessContinents(std::string line, DataType* dataType) {
  // The line is split into separate words
  std::vector<std::string> words = Split(line);

  if (words.size() > 0) {
    // Continents MUST have 2 data points(name and army value).
    if (words.size() < 2) validityData->validData = false;
    // A new continent is generated with the sizes found in the preprocess.
    Continent* continent = new Continent(continentsSizes[index], words[0]);
    // Add the continent to the local container to be later used in the Maps
    // creation
    continents.push_back(continent);
    // Incriment the indicy
    index++;
    try {
    } catch (std::invalid_argument) {
      std::cout << "failure reading integer" << std::endl;
      validityData->validData = false;
    }

  } else {
    // Done processing the continents data, enough data is collected to
    // instantiate the Map object.
    *dataType = DataType::None;
    generatedMap = new Map(territoriesCount, &continents);
  }
}

void MapLoader::ProcessTerritories(std::string line, DataType* dataType) {
  // The line is split into separate words
  std::vector<std::string> words = Split(line);
  if (words.size() > 0) {
    // The line must contain at least 3 data points (indicy, name and
    // continent).
    if (words.size() < 3) validityData->validData = false;
    // A new territoy is created
    std::string* name = new std::string(words[1]);
    Territory* territory = new Territory(name);
    try {
      // The territoy is added to the map
      generatedMap->AddTerritory(territory);
      // continents[std::stoi(words[2]) - 1]->AddTerritory(territory);
      // The territory is added to the continent and vise versa
      territory->SetContinent(continents[std::stoi(words[2]) - 1]);

    } catch (std::invalid_argument) {
      std::cout << "failure reading integer" << std::endl;
      validityData->validData = false;
    }
  } else
    *dataType = DataType::None;
}
void MapLoader::ReadFile(std::string path) {
  // The file must be preprocessed first, if the the file is able to be opened
  // the preprocess begins.
  std::string line;
  std::ifstream myfile(path);
  DataType dataType = DataType::None;
  if (myfile.is_open()) {
    while (getline(myfile, line)) {
      // A regular expression term is used to determine if a new header is found
      // (of the form [XXXXXX]).
      std::regex title("\\[[a-z]+\\]");
      std::smatch m;
      if (regex_search(line, m, title)) {
        // If the regular expression is found, it should be one of the
        // following, and the corresponding validity data is set
        if (m.str().compare("[files]") == 0) {
          validityData->filesFound = true;
        } else if (m.str().compare("[continents]") == 0) {
          dataType = DataType::Continents;
          validityData->continentsFound = true;
        } else if (m.str().compare("[countries]") == 0) {
          dataType = DataType::Territories;
          validityData->territoriesFound = true;
        } else if (m.str().compare("[borders]") == 0) {
          dataType = DataType::Borders;
          validityData->bordersFound = true;
        }
      } else {
        // if a regular expression is not found, then preprocesses should be
        // done of its corresponding data type.
        switch (dataType) {
          case (DataType::Continents):
            PreprocessContinents(line, &dataType);
            break;
          case (DataType::Territories):
            PreprocessTerritories(line, &dataType);
            break;
          case (DataType::Borders):
            PreprocessBorders(line, &dataType);
            break;
        }
      }
    }
    // Close the file and end preprocessing
    myfile.close();
  } else
    std::cout << "Unable to open file";
  // The same files is open from the start to start the main processing and
  // object creation.
  dataType = DataType::None;
  myfile = std::ifstream(path);
  if (myfile.is_open()) {
    while (getline(myfile, line)) {
      std::regex title("\\[[a-z]+\\]");
      std::smatch m;
      if (regex_search(line, m, title)) {
        // If the regular expression is found, it should be one of the
        // following, and the corresponding validity data is set
        if (m.str().compare("[files]") == 0) {
        } else if (m.str().compare("[continents]") == 0) {
          dataType = DataType::Continents;
        } else if (m.str().compare("[countries]") == 0) {
          dataType = DataType::Territories;
        } else if (m.str().compare("[borders]") == 0) {
          dataType = DataType::Borders;
        }
      } else {
        // if a regular expression is not found, then processing should be
        // done of its corresponding data type.
        switch (dataType) {
          case (DataType::Continents):
            ProcessContinents(line, &dataType);
            break;
          case (DataType::Territories):
            ProcessTerritories(line, &dataType);
            break;
          case (DataType::Borders):
            ProcessBorders(line, &dataType);
            break;
        }
      }
    }
    myfile.close();
  } else
    std::cout << "Unable to open file";
}

Map* MapLoader::GenerateMap(std::string filePath) {
  // Call the read file utility function to gather all the data
  ReadFile(filePath);
  // If the data is valid, then we can return the map, otherwise a nullptr is
  // returned.
  if (validityData->IsValid())
    return generatedMap;
  else {
    std::cout << "Warning, data in map file was not valid, a null pointer is "
                 "being returned!!!"
              << std::endl;
    return nullptr;
  }
}

MapLoader::MapLoader() {
  validityData = new ValidityData();
  index = 0;
}

MapLoader::~MapLoader() {
  delete validityData;
  delete[] continentsSizes;
  delete[] neighborsSizes;
}
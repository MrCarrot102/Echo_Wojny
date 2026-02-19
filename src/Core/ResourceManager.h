#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>
#include <string>

class ResourceManager
{
 public:
  void loadTexture(const std::string& name, const std::string& filename)
  {
    sf::Texture tex;
    if (tex.loadFromFile(filename))
    {
      tex.setSmooth(false);
      m_textures[name] = tex;
      std::cout << "[INFO] Zaladowano teksture: " << filename << std::endl;
    }
    else
    {
      std::cerr << "[BLAD] Nie znaleziono pliku: " << filename << std::endl;
    }
  }

  // Pobiera załadowaną teksturę
  sf::Texture& getTexture(const std::string& name)
  {
    return m_textures.at(name);
  }

 private:
  std::map<std::string, sf::Texture> m_textures;
};
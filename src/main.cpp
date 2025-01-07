#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <algorithm>
#include <cmath>
#include <utility>

int main()
{
    auto window = sf::RenderWindow(sf::VideoMode({1800u, 900u}), "Tilemap Padder");
    window.setFramerateLimit(144);
    if (!ImGui::SFML::Init(window))
        return -1;

    std::string texturePath = "";

    sf::Image sourceImg;
    sf::Texture sourceTexture;

    sf::Image targetImg;
    sf::Texture targetTexture;

    sf::Vector2i tileCount{ 7, 5 };
    int currentPadding{ 0 };
    int targetPadding{ 1 };

    bool isDirty = true;

    sf::Clock clock;
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        ImGui::SFML::Update(window, clock.restart());

        ImGui::Begin("Options");
        
        ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, sourceImg.getSize().x ? IM_COL32(255, 255, 255, 255) : IM_COL32(255, 0, 0, 255));
        isDirty = isDirty || ImGui::InputText("Source File", &texturePath);
        ImGui::PopStyleColor();

        isDirty = isDirty || ImGui::InputInt2("Tile Count", &tileCount.x);
        isDirty = isDirty || ImGui::InputInt("Current Padding", &currentPadding);
        isDirty = isDirty || ImGui::InputInt("Target Padding", &targetPadding);
        
        ImGui::End();

        if (isDirty)
        {
            isDirty = false;

            if (!sourceImg.loadFromFile(texturePath))
            {
                sourceImg.resize({}, nullptr);
            }
            else
            {
                sourceTexture.loadFromImage(sourceImg);

                const int tileSize = (sourceImg.getSize().x / tileCount.x) - currentPadding * 2;

                const auto newSize = tileCount * (tileSize + targetPadding * 2);
                targetImg = sf::Image(sf::Vector2u(newSize));

                for (int x = 0; x < tileCount.x; x++)
                {
                    for (int y = 0; y < tileCount.y; y++)
                    {
                        const sf::Vector2i srcPos{ (tileSize + currentPadding * 2) * x + currentPadding, (tileSize + currentPadding * 2) * y + currentPadding };
                        const sf::Vector2i targetPos{ (tileSize + targetPadding * 2) * x + targetPadding, (tileSize + targetPadding * 2) * y + targetPadding };
                        targetImg.copy(sourceImg, sf::Vector2u(targetPos), sf::IntRect(srcPos, { tileSize, tileSize }));

                        for (int z = 0; z < tileSize; z++)
                        {
                            for (int w = 0; w < targetPadding; w++)
                            {
                                targetImg.setPixel(sf::Vector2u(targetPos.x - w - 1, targetPos.y + z), sourceImg.getPixel({ sf::Vector2u(srcPos.x, srcPos.y + z) }));
                                targetImg.setPixel(sf::Vector2u(targetPos.x + tileSize + w, targetPos.y + z), sourceImg.getPixel({ sf::Vector2u(srcPos.x + tileSize - 1, srcPos.y + z) }));
                                targetImg.setPixel(sf::Vector2u(targetPos.x + z, targetPos.y - w - 1), sourceImg.getPixel({ sf::Vector2u(srcPos.x + z, srcPos.y) }));
                                targetImg.setPixel(sf::Vector2u(targetPos.x + z, targetPos.y + tileSize + w), sourceImg.getPixel({ sf::Vector2u(srcPos.x + z, srcPos.y + tileSize - 1) }));
                            }
                        }

                        for (int z = 0; z < targetPadding; z++)
                        {
                            for (int w = 0; w < targetPadding; w++)
                            {
                                targetImg.setPixel(sf::Vector2u(targetPos.x - w - 1, targetPos.y - z - 1), sourceImg.getPixel({ sf::Vector2u(srcPos.x, srcPos.y) }));
                                targetImg.setPixel(sf::Vector2u(targetPos.x + w + tileSize, targetPos.y - z - 1), sourceImg.getPixel({ sf::Vector2u(srcPos.x + tileSize - 1, srcPos.y) }));
                                targetImg.setPixel(sf::Vector2u(targetPos.x - w - 1, targetPos.y + z + tileSize), sourceImg.getPixel({ sf::Vector2u(srcPos.x, srcPos.y + tileSize - 1) }));
                                targetImg.setPixel(sf::Vector2u(targetPos.x + w + tileSize, targetPos.y + z + tileSize), sourceImg.getPixel({ sf::Vector2u(srcPos.x + tileSize - 1, srcPos.y + tileSize - 1) }));
                            }
                        }
                    }
                }

                const std::filesystem::path originalPath(texturePath);
                auto path = originalPath;
                path.replace_extension().replace_filename(path.filename().string() + "_padded").replace_extension(originalPath.extension());

                targetImg.saveToFile(path.string());
                targetTexture.loadFromImage(targetImg);
            }
        }

        window.clear();

        const sf::Vector2f windowSize(window.getSize());
        sf::Vector2f areaSize = windowSize;
        areaSize.x /= 2.f;

        const sf::Vector2f sourceSize(sourceTexture.getSize());
        const sf::Vector2f targetSize(targetTexture.getSize());

        const float sourceRatio = std::min(areaSize.x / sourceSize.x, areaSize.y / sourceSize.y);
        const float targetRatio = std::min(areaSize.x / targetSize.x, areaSize.y / targetSize.y);

        sf::RectangleShape shape{};
        shape.setTexture(&sourceTexture);
        shape.setSize(sourceSize * sourceRatio);
        window.draw(shape);

        shape.setTexture(&targetTexture);
        shape.setSize(targetSize * targetRatio);
        shape.setPosition({ areaSize.x, 0.f });
        window.draw(shape);

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}

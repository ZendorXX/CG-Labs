#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <iostream>

sf::Vector2f calculateBezierPoint(float t, const sf::Vector2f& p0, const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3) {
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    sf::Vector2f p = uuu * p0;
    p += 3 * uu * t * p1;     
    p += 3 * u * tt * p2;    
    p += ttt * p3;           

    return p;
}

float distance(const sf::Vector2f& p1, const sf::Vector2f& p2) {
    return std::sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

sf::Vector2f clampPointToWindow(const sf::Vector2f& point, const sf::RenderWindow& window) {
    float x = std::max(5.0f, std::min(point.x, static_cast<float>(window.getSize().x) - 5.0f));
    float y = std::max(5.0f, std::min(point.y, static_cast<float>(window.getSize().y) - 5.0f));
    return {x, y};
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Кубическая кривая Безье");

    std::vector<sf::CircleShape> controlPoints;
    std::vector<sf::Vector2f> points = {
        {100, 100},
        {200, 500},
        {600, 500},
        {700, 100}
    };

    std::vector<sf::Vector2f> initialPoints = points;

    for (const auto& point : points) {
        sf::CircleShape circle(5);
        circle.setPosition(point - sf::Vector2f(5, 5));
        circle.setFillColor(sf::Color::Red);
        controlPoints.push_back(circle);
    }

    bool dragging = false;
    int selectedPoint = -1;
    
    int numPoints = 100; //int numPoints = 5;

    sf::Clock clock;
    bool animationMode = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::M) {
                    animationMode = !animationMode; 
                    clock.restart();

                    if (animationMode) {
                        initialPoints = points;
                    }

                    std::cout << "Текущий режим: " << (animationMode ? "Анимация" : "Редактирование") << std::endl;

                    std::cout << "Положение точек:" << std::endl;
                    for (const auto& point : points) {
                        std::cout << "(" << point.x << ", " << point.y << ")" << std::endl;
                    }
                }
            }

            if (event.type == sf::Event::Resized) {

                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                window.setView(sf::View(visibleArea));

                for (int i = 0; i < points.size(); ++i) {
                    points[i] = clampPointToWindow(points[i], window);
                    controlPoints[i].setPosition(points[i] - sf::Vector2f(5, 5));
                }
            }

            if (!animationMode) {
                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        for (int i = 0; i < controlPoints.size(); ++i) {
                            sf::Vector2f pos = controlPoints[i].getPosition() + sf::Vector2f(5, 5);
                            if (distance(sf::Vector2f(event.mouseButton.x, event.mouseButton.y), pos) < 15) {
                                dragging = true;
                                selectedPoint = i;
                                break;
                            }
                        }
                    }
                }

                if (event.type == sf::Event::MouseButtonReleased) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        dragging = false;
                        selectedPoint = -1;
                    }
                }

                if (event.type == sf::Event::MouseMoved) {
                    if (dragging && selectedPoint != -1) {
                        float newX = std::max(5.0f, std::min(static_cast<float>(event.mouseMove.x), static_cast<float>(window.getSize().x) - 5.0f));
                        float newY = std::max(5.0f, std::min(static_cast<float>(event.mouseMove.y), static_cast<float>(window.getSize().y) - 5.0f));

                        controlPoints[selectedPoint].setPosition(newX - 5, newY - 5);
                        points[selectedPoint] = {newX, newY};

                        std::cout << "Положение точек:" << std::endl;
                        for (const auto& point : points) {
                            std::cout << "(" << point.x << ", " << point.y << ")" << std::endl;
                        }
                    }
                }
            }
        }

        window.clear();

        for (const auto& circle : controlPoints) {
            window.draw(circle);
        }

        sf::VertexArray curve(sf::LinesStrip, numPoints);
        for (int i = 0; i < numPoints; ++i) {
            float t = static_cast<float>(i) / 99.0f;
            //float t = static_cast<float>(i) / 4.0f; для 4 линий
            sf::Vector2f point = calculateBezierPoint(t, points[0], points[1], points[2], points[3]);
            curve[i].position = point;
            curve[i].color = sf::Color::Blue;
        }
        window.draw(curve);

        if (animationMode) {
            float time = clock.getElapsedTime().asSeconds();
            float t = std::fmod(time, 10.0f) / 10.0f;

            std::cout << "Анимация работает, время: " << time << std::endl;

            points[0] = initialPoints[0] + sf::Vector2f(100 * std::sin(time), 50 * std::cos(time));
            points[1] = initialPoints[1] + sf::Vector2f(100 * std::sin(time * 0.5f), 50 * std::cos(time * 0.5f));
            points[2] = initialPoints[2] + sf::Vector2f(100 * std::sin(time * 0.3f), 50 * std::cos(time * 0.3f));
            points[3] = initialPoints[3] + sf::Vector2f(100 * std::sin(time * 0.7f), 50 * std::cos(time * 0.7f));

            for (int i = 0; i < points.size(); ++i) {
                points[i] = clampPointToWindow(points[i], window);
                controlPoints[i].setPosition(points[i] - sf::Vector2f(5, 5));
            }

            sf::Vector2f animatedPoint = calculateBezierPoint(t, points[0], points[1], points[2], points[3]);
            animatedPoint = clampPointToWindow(animatedPoint, window);
            sf::CircleShape animCircle(5);
            animCircle.setPosition(animatedPoint - sf::Vector2f(5, 5));
            animCircle.setFillColor(sf::Color::Green);
            window.draw(animCircle);
        }

        window.display();
    }

    return 0;
}
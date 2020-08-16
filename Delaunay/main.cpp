#include <random>
#include <chrono>
#include <SFML/Graphics.hpp>
#include "Delaunay.hpp"

int main() {
//    均匀生成随机点
    int numberPoints = 100;
    std::default_random_engine eng(std::random_device{}());
    std::uniform_real_distribution<double> dist_w(0, 800);
    std::uniform_real_distribution<double> dist_h(0, 600);
    cout << "Generating " << numberPoints << " random points" << endl;
    vector<Vector2d> points;
    points.reserve(numberPoints);
    for (int i = 0; i < numberPoints; ++i) {
        points.emplace_back(dist_w(eng), dist_h(eng));
    }

//    三角化
    auto start = std::chrono::system_clock::now();
    Delaunay delaunay(points);
    auto stop = std::chrono::system_clock::now();
    cout << "delaunay: " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count()
         << " seconds\n";

//    SFML 展示
    sf::RenderWindow window(sf::VideoMode(800, 600), "Delaunay triangulation");
    window.clear(sf::Color::Black);
    window.setFramerateLimit(1);

    // Transform each points of each vector as a rectangle
    for (const auto &p : points) {
        sf::RectangleShape s{sf::Vector2f(4, 4)};
        s.setPosition(static_cast<float>(p.x()), static_cast<float>(p.y()));
        window.draw(s);
    }

    std::vector<std::array<sf::Vertex, 2> > lines;
    for (const auto &e : delaunay.getEdges()) {
        const std::array<sf::Vertex, 2> line{{
                                                     sf::Vertex(sf::Vector2f(
                                                             static_cast<float>(e.getU().x() + 2.),
                                                             static_cast<float>(e.getU().y() + 2.))),
                                                     sf::Vertex(sf::Vector2f(
                                                             static_cast<float>(e.getV().x() + 2.),
                                                             static_cast<float>(e.getV().y() + 2.))),
                                             }};
        window.draw(line.data(), 2, sf::Lines);
    }

    window.display();

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
    }

    return 0;
}
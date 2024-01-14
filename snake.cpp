#include <random>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <string>
#include <deque>
#include <map>
#include <set>


using US = unsigned short;

constexpr US SX = 29, SY = 16;
constexpr US HSX = SX / 2, HSY = SY / 2;
constexpr US pxwidth = 16;

constexpr double b = 15;
constexpr unsigned int a = SX * SY, a_sq = a * a;

struct cordT
{
	US x, y;
};


inline const unsigned int f(unsigned int x)
{
	return b / a * sqrt(a_sq - x * x) + 3;
}


inline bool is_free(const cordT& cord, const std::map<US, std::set<US>>& empties)
{
	const auto& set_pair = empties.find(cord.y);
	if (set_pair == empties.end())
		return false;

	const auto& set = set_pair->second;
	if (set.find(cord.x) == set.end())
		return false;
	
	return true;
}


inline void place_apple(cordT& apple, std::map<US, std::set<US>>& empties, unsigned& esize)
{
	unsigned i = rand() % esize;
	for (auto y_ptr = empties.begin(); ; ++y_ptr)
		for (auto x_ptr = y_ptr->second.begin(), x_end = y_ptr->second.end(); x_ptr != x_end; ++x_ptr, --i)
			if (!i)
			{
				empties[apple.y].insert(apple.x);
				apple = { *x_ptr, y_ptr->first };
				empties[apple.y].erase(apple.x);
				return;
			}
}


int WinMain()
{
	// Настройка окна
	//////////////////////////////////////////
	sf::RenderWindow window(sf::VideoMode(SX * pxwidth, SY * pxwidth), "Snake");

	// Установка иконки
	sf::Image icon;
	{
		std::string file = "assets//snake_icon";
		srand(time(0));
		US i = rand();
		if (i % 16 == 0)
			file += "_secret";
		file += ".png";

		if (!icon.sf::Image::loadFromFile(file))
			return EXIT_FAILURE;
	}
	window.setIcon(32, 32, icon.getPixelsPtr());

game_beg:
	window.setFramerateLimit(b);

	// Игровые сущности
	//////////////////////////////////////////
	std::deque<cordT> snake{ {HSX + 1, HSY}, {HSX, HSY}, {HSX - 1, HSY} };
	short direction = 1;
	
	// Словарь пустых клеток
	unsigned esize = SX * SY - 3;
	std::map<US, std::set<US>> empties;
	for (US y = HSY; y; --y)
		for (US x = SX; x; --x)
			empties[y].insert(x);

	for (US y = SY; y != HSY; --y)
		for (US x = SX; x; --x)
			empties[y].insert(x);

	for (US x = 0; x != HSX - 1; ++x)
		empties[HSY].insert(x);
	for (US x = HSX + 2; x <= SX; ++x)
		empties[HSY].insert(x);

	unsigned score = 0;
	cordT apple{1, 1};
	place_apple(apple, empties, esize);

	// Main loop
	//////////////////////////////////////////
	bool is_stopped = false;
	while (window.isOpen())
	{
		// События
		//////////////////////////////////////////
		short chosen_direction = direction;
		
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
				case sf::Event::Closed:
					window.close();
					break;

				case sf::Event::KeyPressed:
					// Escape
					if (event.key.code == sf::Keyboard::Escape) {
						is_stopped = !is_stopped;
						break;
					}
					
					if (is_stopped)
						break;
					
					// Игра работает. Проверка на кнопку управления змейкой
					short considering_direction = chosen_direction;
					switch (event.key.code)
					{
						case sf::Keyboard::A: case sf::Keyboard::Left:
							considering_direction = -1;
							break;
						case sf::Keyboard::W: case sf::Keyboard::Up:
							considering_direction = -2;
							break;
						case sf::Keyboard::D: case sf::Keyboard::Right:
							considering_direction = 1;
							break;
						case sf::Keyboard::S: case sf::Keyboard::Down:
							considering_direction = 2;
							break;
					}

					if (direction + considering_direction != 0 || direction + considering_direction != direction * 2)
						chosen_direction = considering_direction;
					break;
			}
		}

		if (is_stopped)
			continue;

		// Игровой процесс
		//////////////////////////////////////////
		direction = chosen_direction;

		// Перемещение головы змейки
		snake.push_front(*snake.begin());
		cordT& head = *snake.begin();
		switch (direction)
		{
			case 1:
				++head.x;
				break;
			case 2:
				++head.y;
				break;
			case -1:
				--head.x;
				break;
			case -2:
				--head.y;
				break;
		}

		// Продвижение
		if (is_free(head, empties))
		{
			const auto& send = snake.end() - 1;
			const auto& sbeg = *snake.begin();
			empties[send->y].insert(send->x);
			snake.erase(send);
			empties[sbeg.y].erase(sbeg.x);
		}
		else if (head.x == apple.x && head.y == apple.y)
		{
			place_apple(apple, empties, esize);
			++score;
			window.setFramerateLimit(f(score));
		}
		else
		{
			sf::Font font;
			if (!font.loadFromFile("assets//MontereyFLF.ttf"))
				return EXIT_FAILURE;

			sf::Text text_go, text_sc;
			text_go.setFont(font);
			text_sc.setFont(font);

			text_go.setString("Game over");
			text_sc.setString(std::string("Score: ") + std::to_string(score));

			text_go.setCharacterSize(0.2 * SY * pxwidth);
			text_sc.setCharacterSize(0.15 * SY * pxwidth);

			text_go.setFillColor(sf::Color::Black);
			text_sc.setFillColor(sf::Color(255 * sqrt(static_cast<float>(score) / a), 0, 0));

			text_go.setPosition({ HSX * 0.4 * pxwidth, HSY * pxwidth * 0.4 });
			text_sc.setPosition({ HSX * 0.4 * pxwidth, HSY * pxwidth });

			window.setFramerateLimit(5);


			while (window.isOpen())
			{
				// События
				sf::Event event;
				while (window.pollEvent(event))
					switch (event.type)
					{
						case sf::Event::Closed:
							window.close();
							break;
						case sf::Event::KeyPressed:
							if (event.key.code == sf::Keyboard::BackSpace)
								goto game_beg;
					}
					

				// Отрисовка
				window.clear(sf::Color(255, 255, 255));

				window.draw(text_go);
				window.draw(text_sc);

				window.display();
			}
			return EXIT_SUCCESS;
		}

		// Отрисовка
		//////////////////////////////////////////
		window.clear(sf::Color(255, 255, 255));
		
		for (const auto& snake_part : snake) {
			sf::RectangleShape square({ pxwidth, pxwidth });
			square.setFillColor(sf::Color());
			square.setPosition( float(snake_part.x - 1) * pxwidth, float(snake_part.y - 1) * pxwidth);
			window.draw(square);
		}

		sf::RectangleShape apple_square({ pxwidth, pxwidth });
		apple_square.setFillColor(sf::Color(255, 0, 0));
		apple_square.setPosition((apple.x - 1) * pxwidth, (apple.y - 1) * pxwidth);
		window.draw(apple_square);

		window.display();

	}
	return EXIT_SUCCESS;
}

#include <SFML/Graphics.hpp>
#include <iostream>
#include <stack>

using namespace sf;
using namespace std;

RenderWindow window;

float floatReminder(float A, float b) {
	while (A >= b) {
		A -= b;
	}
	return A;
}

class Calc {
private:
	stack <char> operations;
	stack <double> nums;
	char operation_signs[7]{ '+', '-', '*', '/', '^', '(', ')' };

	bool IsOperation(char c) {
		for (auto var : operation_signs)
			if (var == c) return true;
		return false;
	}
	bool IsFalling(char cur, char last) {
		return Rank(cur) <= Rank(last);
	}
	int Rank(char c) {
		if (c == '(') return -1;
		if (c == ')') return 1;
		if (c == '-' || c == '+') return 2;
		if (c == '*' || c == '/') return 3;
		if (c == '^') return 4;
		return 0;
	}
	bool OperationStackIsEmpty() {
		return operations.size() == 0;
	}
	void Fusion(char c) {
		double n2 = nums.top();
		nums.pop();
		double n1 = nums.top();
		nums.pop();
		if (c == '+') nums.push(n1 + n2);
		if (c == '-') nums.push(n1 - n2);
		if (c == '*') nums.push(n1 * n2);
		if (c == '/') nums.push(n1 / n2);
		if (c == '^') nums.push(pow(n1, n2));

		operations.pop();
	}
	string CleanSpaces(string s) {
		string ss = "";
		for (int i = 0; i < s.length(); i++)
		{
			if (s[i] == ' ') continue;
			ss += s[i];
		}
		return ss;
	}
public:
	// TO REVERSE POLISH NOTATION 
	double Calculate(string input) {
		input = CleanSpaces(input);
		string cur_num = "";
		int k = 0;
		for (int i = 0; i < input.size(); i++) {
			char el = input[i];
			if (IsOperation(el)) {
				if (el == '(') {
					operations.push(el);
					continue;
				}
				if (i == 0 && el == '-') {
					cur_num += el;
					continue;
				}
				if (el == '-' && IsOperation(input[i - 1])) {
					cur_num += el;
					continue;
				}
				if (cur_num != "") {
					nums.push(stod(cur_num));
					cur_num = "";
				}
				if (OperationStackIsEmpty()) {
					operations.push(el);
					continue;
				}
				while (IsFalling(el, operations.top())) {
					Fusion(operations.top());
					if (OperationStackIsEmpty()) break;
				}
				if (el == ')') {
					operations.pop();
					continue;
				}
				operations.push(el);
				continue;
			}
			if (el == ',') el = '.'; // Little correction ))
			cur_num += el;
		}
		if (cur_num != "") nums.push(stod(cur_num));
		while (operations.size() != 0) {
			Fusion(operations.top());
		}
		return nums.top();
	}
};

class MathFunction {
public:
	Color color;
	string expression;
	VertexArray Function; //будет выводится на экран у каждой функции

	MathFunction(string expression, Color color) {
		this->color = color;
		Function.setPrimitiveType(Lines); // Устанавливаем тип массива вершин
		string new_expression = "";
		if (expression[0] == '-') new_expression += "0"; // для обработки ситуаций : -(2)^2 [-4] а без этого [4]
		for (int i = 0; i < expression.size(); i++) {
			if (expression[i] == '(' && expression[i + 1] == '-') {
				new_expression += "(0";
				continue;
			}
			new_expression += expression[i];
		}
		this->expression = new_expression;
	}

	float ValueInPoint(float x) { // этот метод нам потребуется также при перестроении значения 
		//находит значение ф-ции в конуретной точке
		string new_expression = "";
		if (expression[0] == '-') new_expression += "0"; // для обработки ситуаций : -(2)^2 [-4] а без этого [4]
		for (int i = 0; i < expression.size(); i++) {
			if (expression[i] == '(' && expression[i + 1] == '-') {
				new_expression += "(0";
				continue;
			}
			if (expression[i] == 'x') {
				if (x < 0) {
					new_expression += "(0";
					new_expression += to_string(x);
					new_expression += ")";
				}
				else {
					new_expression += to_string(x);
				}
				continue;
			}
			new_expression += expression[i];
		}
		Calc result;
		return result.Calculate(new_expression);
	} 
};

class Point {
public:
	float x, y;
	Point() {
		x = 0.f;
		y = 0.f;
	}
	Point(float x, float y) {
		this->x = x;
		this->y = y;
	};

	void move(float dx, float dy) {
		x += dx;
		y -= dy;
	}
};

class Plot {
public:
	Font NumbersFont;
	
	Color DarkLine = Color(50, 50, 50);
	Color LightLine = Color(150, 150, 150);

	vector<Text> Xtexts, Ytexts;		// Текст(числа)
	/*-----------------------------*/
	vector<MathFunction> Collection; //Здесь находятся все графики функций
	/*-----------------------------*/
	VertexArray HorizontalLines;
	VertexArray VerticalLines;
	VertexArray FuncGraph;		//Все точки всех графиков

	MathFunction test = MathFunction("x", Color::Black);	// функция для вычисления dx и dy при скроллинге

	int Width, Height;
	int xcount, ycount;				  // количество  baseSizeX, baseSizeY по ходу программы 
	int FontSize = 35;
	float mx_x = 20, mn_x = 10;			  // max и min для baseSizeX, baseSizeY
	float mx_y = 20, mn_y = 10;			  // max и min для baseSizeX, baseSizeY
	float i_vector, j_vector;		  //значение единичного вектора в пикселях
	float scaleX = 0.5, scaleY = 0.5; //значение единичного вектора в системе
	float baseSizeX, baseSizeY;		  //количество единич знач на Оси
	float sc_x, sc_y;
	float vertex_step = 5;			  //расстояние между точками на графике
	float number_percision = 1;

	Point center;					  // центр сис отсчета


	Plot(int width, int height) {
		xcount = 0;
		ycount = 0;
		baseSizeX = 10.f;
		baseSizeY = 10.f;

		i_vector = width / 10;
		j_vector = height / 10;

		Width = width;
		Height = height;

		window.create(VideoMode(Width, Height), "PLOT MAKER");

		center.x = float(Width / 2.0);
		center.y = float(Height / 2.0);

		NumbersFont.loadFromFile("Dosis-ExtraLight.ttf");

		VerticalLines.setPrimitiveType(Lines);
		HorizontalLines.setPrimitiveType(Lines);
		FuncGraph.setPrimitiveType(Lines);

		FillBaseAxes();
	}
	Text addText(string text, float x_position, float y_position) {
		float k = 1;
		//вычисляем точность единичных отрезков
		if (scaleX >= k/2     || scaleY >= k / 2)   number_percision = 0;
		if (scaleX == k / 4   || scaleY == k / 4)   number_percision = 2;
		if (scaleX == k / 8   || scaleY == k / 8)   number_percision = 3;
		if (scaleX == k / 16  || scaleY == k / 16)  number_percision = 4;
		if (scaleX == k / 32  || scaleY == k / 32)  number_percision = 5;
		if (scaleX == k / 64  || scaleY == k / 64)  number_percision = 6;
		if (scaleX == k / 128 || scaleY == k / 128) number_percision = 7;
		if (scaleX == k / 256 || scaleY == k / 256) number_percision = 8;

		Text tx;
		tx.setFont(NumbersFont);
		tx.setString(text.substr(0, text.find(".") + number_percision));
		tx.setPosition(x_position, y_position);
		tx.setFillColor(Color::White);
		tx.setCharacterSize(FontSize);

		return tx;
	}

	void addFunction(MathFunction f) {
		Collection.push_back(f);
		FillGraph(); // Перезаполняем точки
	}

	void FillGraph() {
		float y;
		/* Заполнение по Ох*/
		FuncGraph.clear();
		
		
		// Проходим по всей оси и в каждой точке Х находим У для каждой из всех функций
		for (float x = -2*Width; x < 2.5*Width; x += vertex_step) { // Проходим по Ох с шагом 1
				for (int i = 0; i < Collection.size(); i++) { //проходим по каждой функции и добавляем точки с соответств. коорд.
					MathFunction func = Collection[i];
					//cout<<"exp: " << Collection[i].expression << endl;
					float v = (x-center.x) / i_vector * scaleX;
					y = -(func.ValueInPoint(v) * j_vector / scaleY - center.y);
					Vertex p,p1;
					
					if (isinf(y)) continue;
					p.position = { x, y };
					p.color = func.color;
					FuncGraph.append(p);

					v = (x+ vertex_step - center.x) / i_vector * scaleX;
					y = -(func.ValueInPoint(v) * j_vector / scaleY - center.y);

					if (isinf(y)) {
						FuncGraph.append(p);
						continue;
					}
					p1.position = { x + vertex_step, y};
					p1.color = func.color;
					FuncGraph.append(p1);
				}
		}
	}

	void FillBaseAxes() {  //Выполняется в начале программы (после запуска) и при изменении масштаба, затем мы просто меняет координаты имеющихся линий и тд
		HorizontalLines.clear();
		VerticalLines.clear();
		Xtexts.clear();
		Ytexts.clear();

		//лучше брать делители сторон экрана
	 	i_vector = (float)(Width / baseSizeX);
		j_vector = (float)(Height / baseSizeY);



		xcount = ycount = 0;	//обнуляем счетчики

		float current_x = floatReminder(center.x, i_vector);
		while (current_x < Width) {
			VerticalLines.append(Vertex(Vector2f(current_x, 0)));
			VerticalLines.append(Vertex(Vector2f(current_x, float(Height))));

			float p = (current_x - center.x) / i_vector; //из сис отсч экрана в базовый сис отсч программы(ОхОу)
			string text = to_string(scaleX * p);		// накидываем scale для надписи
			Text tx;
			tx = addText(text, current_x, center.y);

			// выбираем цвет для i-ой линии
			if ((int)p % 2 != 0) {
				tx.setString("");
				VerticalLines[VerticalLines.getVertexCount() - 2].color = DarkLine;
				VerticalLines[VerticalLines.getVertexCount() - 1].color = DarkLine;
			}
			else {
				VerticalLines[VerticalLines.getVertexCount() - 2].color = LightLine;
				VerticalLines[VerticalLines.getVertexCount() - 1].color = LightLine;
			}
			// если линия центральная
			if (current_x == center.x) {
				tx.setString("");
				VerticalLines[VerticalLines.getVertexCount() - 2].color = Color::Red;
				VerticalLines[VerticalLines.getVertexCount() - 1].color = Color::Red;
			}

			Xtexts.push_back(tx);

			current_x += i_vector;				//увеличиваем текущий Х на базовый вектор(в пикселях)

			xcount++; // количество клеток по Ох (baseSizeX)
		}
		//по аналогии для Оу
		float current_y = floatReminder(center.y, j_vector);
		while (current_y < Height) {
			HorizontalLines.append(Vertex(Vector2f(0, current_y)));
			HorizontalLines.append(Vertex(Vector2f(float(Width), current_y)));

			float p = (-current_y + center.y) / j_vector;
			string text = to_string(scaleY * p);
			Text tx;
			tx = addText(text, center.x+5, current_y);

			if ((int)p % 2 != 0) {
				tx.setString("");
				HorizontalLines[HorizontalLines.getVertexCount() - 2].color = DarkLine;
				HorizontalLines[HorizontalLines.getVertexCount() - 1].color = DarkLine;
			}
			else {
				HorizontalLines[HorizontalLines.getVertexCount() - 2].color = LightLine;
				HorizontalLines[HorizontalLines.getVertexCount() - 1].color = LightLine;
			}
			if (current_y == center.y) {
				HorizontalLines[HorizontalLines.getVertexCount() - 2].color = Color::Red;
				HorizontalLines[HorizontalLines.getVertexCount() - 1].color = Color::Red;
			}

			Ytexts.push_back(tx);

			current_y += j_vector;
			ycount++; //количество клеток по Оу
		}

	}

	void move(float dx, float dy) {  // цель: сдвинуть n столбцов и m строчек на dx и dy соответственно
		center.move(-dx, -dy);		 //сдвигаем начало координат(тк на нем все завязано, но не сильно) и проходимся по каждой точке массива VertexArray
		//newMove(dx);
		for (int i = 0; i < FuncGraph.getVertexCount(); i++) {
			FuncGraph[i].position.x -= dx;
			FuncGraph[i].position.y += dy; 	
		}


		float current_x = floatReminder(center.x, i_vector);
		/*--- ИЗМЕНЕНИЕ ---*/
		// по Оx
		for (int i = 0; i < VerticalLines.getVertexCount(); i += 2) { //i+=2, тк для кадой i мы вручную прописываем две точки : i и (i+1)
			
			float nextX = current_x;
			while (nextX <= 0) nextX += Width;

			VerticalLines[i].position.x = nextX;
			VerticalLines[i+1].position.x = nextX;

			VerticalLines[i].color = nextX == center.x ? Color::Red : LightLine;
			VerticalLines[i+1].color = nextX == center.x ? Color::Red : LightLine;
			
			float p = (nextX - center.x) / i_vector;
			string text = to_string(scaleX *p);
			Text tx;
			tx = addText(text, nextX, center.y);

			if ((int)p % 2 != 0) {
				tx.setString("");
				VerticalLines[i].color = DarkLine;
				VerticalLines[i + 1].color = DarkLine;
			}

			if (nextX == center.x)
				tx.setString("");
			if(center.y < 0)
				tx.setPosition(nextX, 0);
			if (center.y > Height - tx.getCharacterSize())
				tx.setPosition(nextX, Height-tx.getCharacterSize());
			
			Xtexts[(int)i / 2] = tx;

			current_x += i_vector;
		}
		// И точно также для Оу
		float current_y = floatReminder(center.y, j_vector);
		for (int i = 0; i < HorizontalLines.getVertexCount(); i += 2) { //i+=2, тк для кадой i мы вручную прописываем две точки : i и (i+1)

			float nextY = current_y;
			while(nextY <= 0) nextY += Height;
			
			HorizontalLines[i].position.y = nextY;
			HorizontalLines[i + 1].position.y = nextY;

			HorizontalLines[i].color = nextY == center.y ? Color::Red : LightLine;
			HorizontalLines[i + 1].color = nextY == center.y ? Color::Red : LightLine;

			float p = (nextY - center.y) / j_vector;
			string text = to_string(-scaleY * p);
			Text tx;
			tx = addText(text, center.x+5, nextY);

			if ((int)p % 2 != 0) {
				tx.setString("");
				HorizontalLines[i].color = DarkLine;
				HorizontalLines[i + 1].color = DarkLine;
			}

			if (nextY == center.y)
				tx.setString("0");
			if (center.x < 0)
				tx.setPosition(0, nextY);
			if (center.x > Width-40)
				tx.setPosition(Width-40, nextY);
			Ytexts[(int)i / 2] = tx;
			current_y += j_vector;
		}

		/*--- ВЫВОД НА ЭКРАН ---*/
		drawBaseAxes();
		
	}
	void ScaleFillGraph(float scale_dx, float scale_dy) {
		float x, y;

		for (int i = 0; i < FuncGraph.getVertexCount(); i++) {
			x = FuncGraph[i].position.x - center.x;
			y = FuncGraph[i].position.y - center.y;

			x = x * scale_dx;
			y = y * scale_dy;

			FuncGraph[i].position.x = x + center.x;
			FuncGraph[i].position.y = y + center.y;
		}

	}

	void Scale(float Xstep, float Ystep) {
		//изменяется кол-во клеток
		bool reFill = false;

		float _y = 1 / j_vector * scaleY;
		float x = test.ValueInPoint(_y);

		float _x = 1 / i_vector * scaleX;
		float y = test.ValueInPoint(_x);

		sc_y = x;
		sc_x = y;

		baseSizeX -= Xstep;
		baseSizeY -= Ystep;

		//i_vector += Xstep;
		//j_vector += Ystep;

		FillBaseAxes();

		if (xcount > mx_x) {
			baseSizeX = mn_x;
			//i_vector = (float)Width / 10;
			scaleX *= mx_x / mn_x;
			reFill = true;
		}
		else if (xcount < mn_x)
		{
			baseSizeX = mx_x;
			//i_vector = (float)Width / 20;
			scaleX /= mx_x / mn_x;
			reFill = true;
		}

		if (ycount > mx_y) {
			baseSizeY = mn_y;
			//j_vector = (float)Height / 10;
			scaleY *= mx_y / mn_y;
			reFill = true;
		}
		else if (ycount < mn_y)
		{
			baseSizeY = mx_y;
			//j_vector = (float)Height / 20;
			scaleY /= mx_y / mn_y;
			reFill = true;
		}

		if(reFill)
			FillBaseAxes();

		_y = 1 / j_vector * scaleY;
		x = test.ValueInPoint(_y);

		_x = 1 / i_vector * scaleX;
		y = test.ValueInPoint(_x);


		sc_y = sc_y / x;
		sc_x = sc_x / y;


		//FillGraph();
		ScaleFillGraph(sc_x, sc_y);

		drawBaseAxes();

	}

	void drawBaseAxes() {
		window.clear(Color::Black);
		window.draw(HorizontalLines);
		window.draw(VerticalLines);
		window.draw(FuncGraph);
		for (int i = 0; i < Xtexts.size(); i++)
			window.draw(Xtexts[i]);
		for (int i = 0; i < Ytexts.size(); i++)
			window.draw(Ytexts[i]);
		window.display();
	}
};

int main()
{
	Plot a = Plot(1000,1000);

	MathFunction f = MathFunction("2^(x)", Color::Blue);
	a.addFunction(f);

	MathFunction f2 = MathFunction("2+1/(x+5)", Color::Green);
	a.addFunction(f2);
	
	MathFunction f3 = MathFunction("x^(1/2)", Color::Yellow);
	a.addFunction(f3);

	Color PINK = Color(255, 0, 234);
	MathFunction f4 = MathFunction("-0.5*x", PINK);
	a.addFunction(f4);

	Color PURPULE = Color(200, 108, 240);
	MathFunction f5 = MathFunction("(1-(x-3)^2)^(1/2)", PURPULE); 
	a.addFunction(f5);

	MathFunction f6 = MathFunction("-(1-(x-3)^2)^(1/2)", PURPULE);
	a.addFunction(f6);

	Color LightGreen = Color(68, 150, 98);
	MathFunction f7 = MathFunction("0.25*x^2", LightGreen);
	a.addFunction(f7);


	a.FillGraph();
	a.drawBaseAxes();
	
	float move_step = 4.f;
	float scale_step = 1.f;

	while(window.isOpen())
	{
		Event event;


		while (window.pollEvent(event))
		{

			if(event.type == Event::Closed)
				window.close();
			if (event.type == Event::TextEntered && event.text.unicode == 43) {
				// '+'
				a.Scale(10,10);
			}
			else if (event.type == Event::TextEntered && event.text.unicode == 45) {
				// '-'
				a.Scale(-10,-10);
			}
			else if (event.type == sf::Event::MouseWheelMoved)
			{
				a.Scale(5* event.mouseWheel.delta, 5 * event.mouseWheel.delta);
			}
			else if (Mouse::isButtonPressed(Mouse::Left)) { // Перемещение с помощью мышки
				float x, y;
				//начальное положение при нажатии
				float x0 = Mouse::getPosition().x; 
				float y0 = Mouse::getPosition().y;
				while (Mouse::isButtonPressed(Mouse::Left)) // тянем мышку
				{
					//каждый раз конечное положение изменяется
					x = Mouse::getPosition().x;
					y = Mouse::getPosition().y;

					float dx = x - x0, dy = y - y0;
					// Подсчитываем разницу и смещаем всю систему
					a.move(-dx, dy);
					//сдвигаем начальное значение на конечное. Затем весь цикл повторяется
					x0 += dx;
					y0 += dy;
				}
			}
			if (Keyboard::isKeyPressed(Keyboard::W)) {
				a.Scale(0, scale_step);
			}
			if (Keyboard::isKeyPressed(Keyboard::S)) {
				a.Scale(0, -scale_step);
			}
			if (Keyboard::isKeyPressed(Keyboard::A)) {
				a.Scale(-scale_step, 0);
			}
			if (Keyboard::isKeyPressed(Keyboard::D)) {
				a.Scale(scale_step, 0);
			}
			if (Mouse::isButtonPressed(Mouse::Button::Right)) {
				a.FillGraph();
				a.drawBaseAxes();
			}
		}
		
		if (Keyboard::isKeyPressed(Keyboard::Up)) {
			a.move(0, move_step);
		}
		if (Keyboard::isKeyPressed(Keyboard::Down)) {
			a.move(0, -move_step);
		}
		if (Keyboard::isKeyPressed(Keyboard::Left)) {
			a.move(-move_step, 0);
		}
		if (Keyboard::isKeyPressed(Keyboard::Right)) {
			a.move(move_step, 0);
		}

	}

	return 0;
}

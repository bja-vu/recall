#include "crow.h"

int main() {
	crow::SimpleApp app;

	CROW_ROUTE(app, "/")([](){
		return "hello world";
	});

	app.port(8000).multithreaded().run();

}

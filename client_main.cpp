#include <iostream>
#include "message_broker.hpp"
#include <chrono>
#include <thread>

const char *message = "{\"config\":{\"config_name\":\"ip_address\",\"config_value\":\"192.168.1.1\"}}";
const char *query1 = "{\"query_name\":\"get-user-by-id\",\"query_data\":{\"id\":1}}";
const char *query2 = "{\"query_name\":\"get-user-by-id\",\"query_data\":{\"id\":3}}";


int main(int argc, char const *argv[])
{
	MessageBroker broker;


	broker.publish("amq.direct", "test", message);
	/*broker.publish("amq.direct", "test", query1, [](const auto &response) {
		if (response.ok()) {
			std::cout << response.serializeBody() << std::endl;
		} else {
			std::cout << response.reason() << std::endl;
		}
	});

	broker.publish("amq.direct", "test2", query1, [](const auto &response) {
		if (response.ok()) {
			std::cout << response.serializeBody() << std::endl;
		} else {
			std::cout << response.reason() << std::endl;
		}
	});*/

	while(1){}
	/*for (int i = 0; i < 10; ++i) {
        std::cout << '.'; std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }*/

	/*broker.publish(
		MessageBroker::Exchange("amq.direct"),
		MessageBroker::Queue(),
		"logs",
		"hello"
	); */

	/*broker.subsribe(
		MessageBroker::Exchange("logs", "direct"),
		MessageBroker::Queue(""),
		[](const VistaBroker::Request &request, VistaBroker::Reponse &response) {

		}
	)*/

}

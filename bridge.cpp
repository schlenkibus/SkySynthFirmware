#include <iostream>
#include <iostream>
#include <string>
#include <regex>
#include <iterator>

#define HEADSOCKET_IMPLEMENTATION
#include "HeadSocket.h"
#include "tynosc.hpp"
#include "StringTools.h"
#include <SFML\Network.hpp>

using namespace headsocket;

class client : public web_socket_client
{
	HEADSOCKET_CLIENT(client, web_socket_client)
		sf::UdpSocket socket;
public:
	bool async_received_data(const data_block &db, uint8_t *ptr, size_t length) override
	{

		auto keyValue = StringTools::splitStringOnAnyDelimiter(std::string(ptr, ptr + length), '#');
		tnyosc::Message msg("/" + keyValue[0]);
		auto value = std::atof(keyValue[1].c_str());
		std::cout << "send value: " << value << '\n';
		msg.append((float)value);
		//tnyosc::Bundle bundle;
		//bundle.append(msg);
		int32_t send_size = msg.size();
		std::cout << std::string(msg.data(), msg.size()) << '\n';
		socket.send(msg.data(), send_size, sf::IpAddress::LocalHost, 3333);
		return true;
	}
};

int main()
{
	typedef web_socket_server<client> server_t;
	auto server = server_t::create(3334);
	while (server->is_running())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	return 0;
}

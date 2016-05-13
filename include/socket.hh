#ifndef _SOCKET_HH_
#define _SOCKET_HH_

#include <string>
#include <net/ethernet.h> /* the L2 protocols */
#include <netpacket/packet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h> /* struct ifreq */

class Socket {
public:
	Socket(const char * device);
	virtual ~Socket();

	int get_descriptor(void);

	int send_message(const unsigned char *destination_address,
			const char *message, int message_length);
	int receive_message(unsigned char *source_address,
			char *buffer, int buffer_size);

	enum MembershipType {
		MULTICAST = PACKET_MR_MULTICAST,
		PROMISCIOUS = PACKET_MR_PROMISC,
		ALL_MULTICAST = PACKET_MR_ALLMULTI
	};

	void add_membership(Socket::MembershipType type, const unsigned char *multicast_address);
private:
	std::string device_name;
	int socket_descriptor;
	int interface_index;
	unsigned char hardware_address[ETHER_ADDR_LEN];

	Socket(const Socket& that);
	Socket& operator=(const Socket& that);

	enum MembershipAction {
		ADD_MEMBERSHIP = PACKET_ADD_MEMBERSHIP,
		DROP_MEMBERSHIP = PACKET_DROP_MEMBERSHIP
	};

	void manage_membership(Socket::MembershipAction action,
			Socket::MembershipType type, const unsigned char *multicast_address);
};

#endif

#ifndef LEVIATHAN_CONNECTIONINFO
#define LEVIATHAN_CONNECTIONINFO
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "NetworkResponse.h"
#include "NetworkRequest.h"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/UdpSocket.hpp"
#include "SFML/Network/IpAddress.hpp"
#include "Common/ThreadSafe.h"
#include <boost/thread/future.hpp>

namespace Leviathan{

	struct MadeNetworkRequest{

		MadeNetworkRequest(int expectedresponseid, shared_ptr<NetworkRequest> request, shared_ptr<boost::promise<bool>> waitobject, int maxtries, int timeout, int attempnumber = 1);

		int MaxTries;
		int AttempNumber;
		int TimeOutMS;
		__int64 RequestStartTime;
		__int64 ResponseReceiveTime;
		int ExpectedResponseID;
		shared_ptr<boost::promise<bool>> WaitForMe;
		shared_ptr<NetworkResponse> GotResponse;
		// The original request //
		shared_ptr<NetworkRequest> OriginalRequest;
	};


	class ConnectionInfo : public ThreadSafe{
	public:
		DLLEXPORT ConnectionInfo(shared_ptr<wstring> hostname, USHORT port = sf::Socket::AnyPort);
		DLLEXPORT ~ConnectionInfo();

		// Creates the socket //
		DLLEXPORT bool Init();
		// Closes the socket //
		DLLEXPORT void ReleaseSocket();

		DLLEXPORT void UpdateListening();

		DLLEXPORT shared_ptr<MadeNetworkRequest> SendPacketToConnection(shared_ptr<NetworkRequest> request, int maxretries);

		// Data exchange functions //
		DLLEXPORT shared_ptr<NetworkResponse> SendRequestAndBlockUntilDone(shared_ptr<NetworkRequest> request, int maxtries = 2);


	private:

		void _PopMadeRequest(shared_ptr<MadeNetworkRequest> objectptr, ObjectLock &guard);
		void _ResendRequest(shared_ptr<MadeNetworkRequest> toresend, ObjectLock &guard);

		// ------------------------------------ //

		// Potentially queue requests //


		// Made requests that are waiting //
		std::list<shared_ptr<MadeNetworkRequest>> WaitingRequests;


		sf::UdpSocket _Socket;

		USHORT PortNumber;
		USHORT TargetPortNumber;
		shared_ptr<wstring> HostName;
		sf::IpAddress TargetHost;
	};

}
#endif
#ifndef SDL_NET_H_INCLUDED
#define SDL_NET_H_INCLUDED

struct contact // a struct that holds client contact info. a seomwhat serialized form of client
{
    int port;//port is this contact's port. serv is the port the server uses to connect to the client
   std:: string name;
    Paper sheet, moves; // two paper objects to handle two sprites
    contact(std::string title, int p )
    {
        port = p;
        name = title;
        //address = a;
    }
    std::string packet()
    {
        std::ostringstream os;
        os << port;
        return os.str() + "|" + name;
    }

};

 bool send(UDPpacket* pack,UDPsocket sock, int otherPort,  const char* address, std::string message)
    {
       // UDPpacket* pack = SDLNet_AllocPacket(512);
        pack->len = message.length();
        memcpy(pack->data, message.c_str(), message.length());
        IPaddress ip;
        SDLNet_ResolveHost(&ip, address, otherPort );
        pack->address.host = ip.host;
        pack->address.port = ip.port;
      //  std::cout << "Sent: " << pack->data << std::endl;
 //     std::cout << &sock << std::endl;
   // std::cout << otherPort << std::endl;
   //std::cout << &pack << std::endl;
       if ( SDLNet_UDP_Send(sock, -1, pack) == 0)
       {
          std:: cout << "Error sending data: " << SDLNet_GetError( ) << std::endl;
           return false;
       }
       else
       {
          return true;
       }

    }

#endif // SDL_NET_H_INCLUDED

#pragma once
#include <memory>

typedef int UIDTy;

class WSUser;
typedef std::shared_ptr<WSUser> WSUserPtr;

class Session;
typedef std::shared_ptr<Session> SessionPtr;

class AudioPacket;
typedef std::shared_ptr<AudioPacket> AudioPacketPtr;

class SpeakerStream;
typedef std::shared_ptr<SpeakerStream> SpeakerStreamPtr;

class Server;
class ServerLockedGuards;
class ServerLockedGuardsDrop;
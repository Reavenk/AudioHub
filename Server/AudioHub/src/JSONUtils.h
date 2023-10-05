#pragma once

#include "vendored/nlohmann/json.hpp"
using json = nlohmann::json;

#define JSHS_REQLOGIN		"reqlogin"

#define JSREQ_LOGIN			"login"
#define JSREQ_LOGOUT		"logout"
#define JSREQ_VOLUME		"volume"
#define JSREQ_GATE			"gate"
#define JSREQ_CHSESSION		"chsession"

#define JSKEY_ACTION		"action"
#define JSKEY_DATA			"data"

#define JSNOTIF_USERENTER	"userenter"
#define JSNOTIF_USERLEFT	"userleft"
#define JSNOTIF_BROADCAST	"broadcast"
#define JSNOTIF_FORWARD		"forward"
#define JSNOTIF_DISCONNECT	"disconnect"
#define JSNOTIF_REBUFFER	"rebuffer"

#define STRING_SUCCESS		"success"
#define STRING_ERROR		"error"

#undef CreateEvent

namespace JSUtils
{
	json CreateMessage(const std::string& type, const std::string& evt, const json& data, const std::string& status, const std::string& error = "");

	// Responses are sent in response to a request. The event will be the same as what was passed in.
	json CreateResponse(const std::string& event, const json& data);
	// A handshake message are messages that involve multiple message to accomplish a task. Or, a request
	// from the server that expects a response from the client.
	json CreateHandshake(const std::string& event, const json& data);
	// Error is an error message send from the server. 
	json CreateError(const std::string& event, const std::string& error, const json& data);
	// An event is a message that is sent from the server in response to something outside of what
	// is happening in direct communication between the server and client.
	json CreateEvent(const std::string& event, const json& data);
	
	std::string ExtractExpectedString(const std::string& key, const json& js);
	bool ExtractExpectedBool(const std::string& key, const json& js);
	float ExtractExpectedFloat(const std::string& key, const json& js);
	int ExtractExpectedInt(const std::string& key, const json& js);
	json ExtractExpectedJSON(const std::string& key, const json& js);

	bool ExtractString(const std::string& key, const json& js, std::string& out);
	bool ExtractBool(const std::string& key, const json& js, bool& out);
	bool ExtractFloat(const std::string& key, const json& js, float& out);
	bool ExtractInt(const std::string& key, const json& js, int& out);
	const json* ExtractJSON(const std::string& key, const json& js);
	
	void ExtractActionAndData(const json& js, std::string& outAction, json& data);
}
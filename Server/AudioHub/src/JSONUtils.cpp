#include "JSONUtils.h"
#include "WSUser.h"

#undef CreateEvent

namespace JSUtils
{
	json CreateMessage(const std::string& type, const std::string& evt, const json& data, const std::string& status, const std::string& error)
	{
		json ret;
		ret["type"]		= type;
		ret["event"]	= evt;
		ret["data"]		= data;
		ret["status"]	= status;

		if (!error.empty())
			ret["error"] = error;

		return ret;
	}

	json CreateResponse(const std::string& event, const json& data)
	{
		return CreateMessage("response", event, data, STRING_SUCCESS );
	}

	json CreateHandshake(const std::string& event, const json& data)
	{
		return CreateMessage("handshake", event, data, STRING_SUCCESS );
	}

	json CreateError(const std::string& event, const std::string& error, const json& data)
	{
		return CreateMessage("error", event, data, STRING_ERROR, error);
	}

	json CreateEvent(const std::string& event, const json& data)
	{
		return CreateMessage("event", event, data, STRING_SUCCESS);
	}

	std::string ExtractExpectedString(const std::string& key, const json& js)
	{
		if(!js.contains(key))
			throw std::runtime_error("Expected key not found in data.");
		
		if(!js[key].is_string())
			throw std::runtime_error("Expected key is not a string.");
		
		return js[key];
	}

	bool ExtractExpectedBool(const std::string& key, const json& js)
	{
		if(!js.contains(key))
			throw std::runtime_error("Expected key not found in data.");

		if(!js[key].is_boolean())
			throw std::runtime_error("Expected key is not a bool.");

		return js[key];
	}

	float ExtractExpectedFloat(const std::string& key, const json& js)
	{
		if(!js.contains(key))
			throw std::runtime_error("Expected key not found in data.");

		if(!js[key].is_number())
			throw std::runtime_error("Expected key is not a number.");

		return js[key];
	}

	int ExtractExpectedInt(const std::string& key, const json& js)
	{
		if(!js.contains(key))
			throw std::runtime_error("Expected key not found in data.");

		if(!js[key].is_number())
			throw std::runtime_error("Expected key is not a number.");

		return js[key];
	}

	json ExtractExpectedJSON(const std::string& key, const json& js)
	{
		if(!js.contains(key))
			throw std::runtime_error("Expected key not found in data.");

		if(!js[key].is_object())
			throw std::runtime_error("Expected key is not an object.");

		return js[key];
	}

	bool ExtractString(const std::string& key, const json& js, std::string& out)
	{
		if (!js.contains(key))
			return false;

		if (!js[key].is_string())
			return false;

		out = js[key];
		return true;
	}
	
	bool ExtractBool(const std::string& key, const json& js, bool& out)
	{
		if (!js.contains(key))
			return false;

		if (!js[key].is_boolean())
			return false;

		out = js[key];
		return true;
	}

	bool ExtractFloat(const std::string& key, const json& js, float& out)
	{
		if (!js.contains(key))
			return false;

		if (!js[key].is_number())
			return false;

		out = js[key];
		return true;
	}

	bool ExtractInt(const std::string& key, const json& js, int& out)
	{
		if (!js.contains(key))
			return false;

		if (!js[key].is_number())
			return false;

		out = js[key];
		return true;
	}

	const json* ExtractJSON(const std::string& key, const json& js)
	{
		if (!js.contains(key))
			return nullptr;

		if (!js[key].is_object())
			return nullptr;

		return &js[key];
	}

	void ExtractActionAndData(const json& js, std::string& outAction, json& data)
	{
		outAction = JSUtils::ExtractExpectedString(JSKEY_ACTION, js);

		if (!js.contains(JSKEY_DATA))
			throw std::runtime_error("Missing data field in message.");

		data = js[JSKEY_DATA];
	}
}
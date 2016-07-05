#include <netdb.h>
#include "profile.h"
#include "util.h"
using namespace std;

namespace bcm2dump {
namespace {
string& unescape(string& str, char delim)
{
	string::size_type i = 0;
	while ((i = str.find('\\', i)) != string::npos) {
		if ((i + 1) >= str.size()) {
			throw invalid_argument("stray backslash in '" + str + "'");
		} else if (str[i + 1] == '\\' || str[i + 1] == delim) {
			str.erase(i, 1);
			i += 1;
		} else {
			throw invalid_argument("invalid escape sequence in '" + str + "'");
		}
	}

	return str;
}

void push_back(vector<string>& strings, string str, char delim, bool empties, size_t limit)
{
	if (empties || !str.empty()) {
		if (!limit || strings.size() < limit) {
			strings.push_back(unescape(str, delim));
		} else {
			strings.back() += delim + unescape(str, delim);
		}
	}
}

bool is_unescaped(const string& str, string::size_type pos)
{
	if (!pos) {
		return true;
	}

	bool ret = false;

	if (pos >= 1) {
		ret = str[pos - 1] != '\\';
	}

	if (pos >= 2 && !ret) {
		ret = str[pos - 2] == '\\';
	}

	return ret;
}

}

string trim(string str)
{
	if (str.empty()) {
		return str;
	}

	auto i = str.find_last_not_of(" \x0d\n\t");
	if (i != string::npos) {
		str.erase(i + 1);
	}

	i = str.find_first_not_of(" \x0d\n\t");
	if (i == string::npos) {
		return "";
	}

	return str.substr(i);
}

vector<string> split(const string& str, char delim, bool empties, size_t limit)
{
	string::size_type beg = 0, end = str.find(delim);
	vector<string> subs;

	while (end != string::npos) {
		if (is_unescaped(str, end)) {
			push_back(subs, str.substr(beg, end - beg), delim, empties, limit);
			beg = end + 1;
		}

		end = str.find(delim, end + 1);

		if (end == string::npos) {
			push_back(subs, str.substr(beg, str.size()), delim, empties, limit);
		}
	}

	if (subs.empty() && !str.empty()) {
		push_back(subs, str, delim, empties, limit);
	}

	return subs;
}

string to_hex(const std::string& buffer)
{
	string ret;

	for (char c : buffer) {
		ret += to_hex(c & 0xff, 2);
	}

	return ret;
}

uint16_t crc16_ccitt(const void* buf, size_t size)
{
	uint32_t crc = 0xffff;
	uint32_t poly = 0x1021;

	for (size_t i = 0; i < size; ++i) {
		for (size_t k = 0; k < 8; ++k) {
			bool bit = (reinterpret_cast<const char*>(buf)[i] >> (7 - k) & 1);
			bool c15 = (crc >> 15 & 1);
			crc <<= 1;
			if (c15 ^ bit) {
				crc ^= poly;
			}
		}
	}

	return crc & 0xffff;
}

/*
uint32_t crc32(const string& buf)
{
	uint32_t poly = 0xedb88320;
	uint32_t crc = 0;

	for (size_t i = 0; i < buf.size(); ++i) {
		uint8_t a = (crc & 0xff) ^ buf[i];
		for (size_t k = 0; k < 8; ++k) {
			a = ((a & 1) ? 0 : poly) ^ (a >> 1);
		}
		a ^= 0xff000000;


	}
	return crc<uint32_t, 0x4b0bbe37>(buf.data(), buf.size(), 0xffffffff) ^ 0xffffffff;
}
*/

ofstream logger::s_bucket;
int logger::s_loglevel = logger::info;

constexpr int logger::trace;
constexpr int logger::debug;
constexpr int logger::verbose;
constexpr int logger::info;
constexpr int logger::warn;
constexpr int logger::err;

ostream& logger::log(int severity)
{
	if (severity < s_loglevel) {
		return s_bucket;
	} else if (severity >= warn) {
		return cerr;
	} else {
		return cout;
	}
}

string getaddrinfo_category::message(int condition) const
{
	return gai_strerror(condition);
}
}

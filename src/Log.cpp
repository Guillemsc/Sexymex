#define _CRT_SECURE_NO_WARNINGS

#include "Log.h"
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

// Array of strings with the names of LogLevels
static const char *s_LevelStrings[] = {
	"NONE",
	"ERROR",
	"WARNING",
	"INFO",
	"DEBUG",
	"ALL"
};

#ifdef _WIN32
#define SEPARATOR '\\'
#else
#define SEPARATOR '/'
#endif
#define BASENAME(file) (strrchr(file, SEPARATOR) ? strrchr(file, SEPARATOR) + 1 : file)

// Global Log instance /////////////////////////////////////////////////

Log g_Log;


// LogMessage //////////////////////////////////////////////////////////

LogMessage::LogMessage(Log *owner) :
	_owner(owner), _file(0), _line(0), _level(LDebug)
{ }

LogMessage::LogMessage(Log *owner, const char *file, int line) :
	_owner(owner), _file(BASENAME(file)), _line(line), _level(LDebug)
{ }

LogMessage::~LogMessage()
{
	_owner->flush(*this);
}

const std::string &LogMessage::str() const
{
	return _buffer;
}

const char *LogMessage::file() const
{
	return _file;
}

int LogMessage::line() const
{
	return _line;
}

LogLevel LogMessage::level() const
{
	return _level;
}

LogMessage& LogMessage::operator<<(const std::string& text)
{
	_buffer.append(text);
	return *this;
}

LogMessage& LogMessage::operator<<(int i)
{
	std::ostringstream ss; ss << i;
	_buffer.append(ss.str());
	return *this;
}

LogMessage& LogMessage::operator<<(unsigned int i)
{
	std::ostringstream ss; ss << i;
	_buffer.append(ss.str());
	return *this;
}

LogMessage& LogMessage::operator<<(float f)
{
	std::ostringstream ss; ss << f;
	_buffer.append(ss.str());
	return *this;
}

LogMessage& LogMessage::operator<<(double d)
{
	std::ostringstream ss; ss << d;
	_buffer.append(ss.str());
	return *this;
}

LogMessage& LogMessage::operator<<(LogLevel l)
{
	_level = l;
	return *this;
}


// Log /////////////////////////////////////////////////////////////////

Log::Log() :
	_cout(true),
	_verbosity(LAll)
{ }

Log::~Log()
{ }

void Log::enableConsoleOutput(bool enable)
{
	_cout = enable;
}

bool Log::enableFileOutput(const std::string& filepath)
{
	std::fstream file(filepath.c_str(), std::ios::out | std::ios::trunc);
	bool is_open = file.is_open();
	if (is_open) { _filename = filepath; }
	return is_open;
}

void Log::addOutput(LogOutput *output)
{
	_outputs.push_back(output);
}

void Log::setVerbosity(LogLevel level)
{
	_verbosity = level;
}

LogMessage Log::operator()()
{
	return LogMessage(this);
}

LogMessage Log::operator()(const char *file, int line)
{
	return LogMessage(this, file, line);
}

void Log::flush(const LogMessage &m)
{
	const std::string &text(m.str());
	if (_verbosity >= m.level())
	{
		char wholeText[1024];
		char fileLine[512];

		const char *srcfile = m.file();
		int srcline = m.line();
		const char *lvlstr = s_LevelStrings[m.level()];
		if (srcfile) {
			sprintf(fileLine, "%s:%d", srcfile, srcline);
			const int paddingCount = 7 - (int)strlen(lvlstr);
			char padding[7] = { ' ',' ',' ',' ',' ',' ',' ' };
			padding[paddingCount] = '\0';
			//sprintf(wholeText, "<%s>%s | %s       (%s)\n", lvlstr, padding, text.c_str(), fileLine);
			int timestamp = GetTickCount();
			sprintf(wholeText, "%06d <%s>%s | %s\n", timestamp, lvlstr, padding, text.c_str());
		}
		else {
			sprintf(wholeText, "<%s> - %s\n", lvlstr, text.c_str());
		}

		if (!_filename.empty()) {
			std::fstream file(_filename.c_str(), std::ios::out | std::ios::app);
			file << wholeText << std::flush;
		}
		if (_cout) {
			std::cout << wholeText << std::flush;
		}
		for (auto output : _outputs) {
			output->writeMessage(wholeText);
		}
	}
}

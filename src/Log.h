#ifndef M_LOG_H
#define M_LOG_H

#include <string>
#include <vector>


#define eLog g_Log(__FILE__, __LINE__) << LError
#define wLog g_Log(__FILE__, __LINE__) << LWarn
#define iLog g_Log(__FILE__, __LINE__) << LInfo
#define dLog g_Log(__FILE__, __LINE__) << LDebug


// Forward declaration
class Log;

/** Enumerated type for log levels. */
enum LogLevel { LNone, LError, LWarn, LInfo, LDebug, LAll };


/***********************************************************************
* LogMessage class.
* Class that just contains a buffer to accumulate the entire message.
**********************************************************************/
class LogMessage
{
public:

	// Destructor
	~LogMessage();

	// Message information
	const std::string &str() const;
	const char *file() const;
	int line() const;
	LogLevel level() const;

	// Basic output operators
	LogMessage& operator<<(const std::string& text);
	LogMessage& operator<<(int i);
	LogMessage& operator<<(unsigned int i);
	LogMessage& operator<<(float f);
	LogMessage& operator<<(double d);
	LogMessage& operator<<(LogLevel l);

private:

	// Private constructor
	LogMessage(Log *owner);
	LogMessage(Log *owner, const char *file, int line);
	friend class Log;

	// Private attributes
	Log *_owner; /**< Log instance that created this message. */
	std::string _buffer; /**< User defined log message. */
	const char *_file; /**< File where the log is requested. */
	int _line; /**< Line where the log is requested. */
	LogLevel _level; /**< Message verbosity level. */
};


/***********************************************************************
* LogOutput interface.
**********************************************************************/
class LogOutput
{
public:
	LogOutput() { }
	virtual ~LogOutput() { }

	virtual void writeMessage(const std::string &message) = 0;
};


/***********************************************************************
* Logging class.
**********************************************************************/
class Log
{
private:

	////////////////////////////////////////////////////////////////////
	// Private members
	////////////////////////////////////////////////////////////////////

	bool _cout; /**< Whether or not log to std::cout. */
	std::string _filename; /**< Output file. */
	LogLevel _verbosity; /**< Log verbosity level. */
	std::vector<LogOutput*> _outputs; /**< Array of LogOutput objects. */


public:

	////////////////////////////////////////////////////////////////////
	// Constructor and destructor
	////////////////////////////////////////////////////////////////////

	/** Constructor. */
	Log();

	/** Destructor. */
	~Log();


	////////////////////////////////////////////////////////////////////
	// Configuration methods
	////////////////////////////////////////////////////////////////////

	/**
	* It sets whether or not the log will write on console output.
	* @param enable true to print on console, false otherwise.
	*/
	void enableConsoleOutput(bool enable);

	/**
	* It sets whether or not the log will write in a file.
	* @param	file Output file path.
	* @return True if the file can be opened for writing.
	*/
	bool enableFileOutput(const std::string& file);

	/**
	 * It adds an extra output for the logging.
	 */
	void addOutput(LogOutput *output);

	/**
	* It sets the verbosity for this Log.
	* @param Level of verbosity.
	*/
	void setVerbosity(LogLevel level);

	////////////////////////////////////////////////////////////////////
	// Logging methods
	////////////////////////////////////////////////////////////////////

	/**
	* It returns a new, temporary message to log into.
	* @return A temporary message to fill before flushing.
	*/
	LogMessage operator()();

	/**
	* It returns a new, temporary message to log into.
	* @param file File this function is invoked from.
	* @param line Line this function is invoked from.
	* @return A temporary message to fill before flushing.
	*/
	LogMessage operator()(const char *file, int line);

	/**
	* It flushes the message content onto the logging stream.
	* @param m A log message that has already been filled.
	*/
	void flush(const LogMessage &m);
};


////////////////////////////////////////////////////////////////////////
// Global vars
////////////////////////////////////////////////////////////////////////

/** Global Log instance - m::log */
extern Log g_Log;



#endif // M_LOG_H

#ifndef CONFIGEXCEPTION_H
#define CONFIGEXCEPTION_H

#include <string>

class ConfigException : virtual public std::exception
{

private:
    std::string message;

public:
    ConfigException(std::string message) : message(message) {}
    ConfigException(const char *message) : message(message) {}
    ~ConfigException() {}

    inline virtual const char *what() const throw() override
    {
        return this->message.c_str();
    }
};

#endif // CONFIGEXCEPTION_H
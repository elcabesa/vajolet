/*
	This file is part of Vajolet.

    Vajolet is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Vajolet is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Vajolet.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef UCI_OPTION_H_
#define UCI_OPTION_H_

#include <string>

#include "uciManagerImpl.h"

class UciOption
{
public:
	virtual bool setValue( const std::string& v, bool verbose = true) = 0;
	virtual std::string print() const = 0;
	virtual ~UciOption(){}
	bool operator==(const std::string& rhs);
protected:
	UciOption(const std::string& name);
	const std::string _name;
};

class StringUciOption final: public UciOption
{
public:
	StringUciOption(const std::string& name, std::string& value, UciManager::impl* uci, void (UciManager::impl::*callbackFunc)(std::string), const std::string& defVal);
	std::string print() const override;
	bool setValue( const std::string& s, bool verbose = true) override;
private:
	const std::string _defaultValue;
	std::string& _value;
	void (UciManager::impl::*_callbackFunc)(std::string);
	UciManager::impl* _uci;
};

class SpinUciOption final: public UciOption
{
public:
	SpinUciOption( const std::string& name, unsigned int& value, UciManager::impl* uci, void (UciManager::impl::*callbackFunc)(unsigned int), const unsigned int defVal, const unsigned int minVal, const int unsigned maxVal);
	std::string print() const override;
	bool setValue( const std::string& s, bool verbose = true) override;		
private:
	const unsigned int _defValue;
	const unsigned int _minValue;
	const unsigned int _maxValue;
	unsigned int & _value;
	void (UciManager::impl::*_callbackFunc)(unsigned int);
	UciManager::impl* _uci;
};

class CheckUciOption final: public UciOption
{
public:
	CheckUciOption( const std::string& name, bool& value, UciManager::impl* uci, void (UciManager::impl::*callbackFunc)(bool), const bool defVal);
	std::string print() const override;
	bool setValue( const std::string& s, bool verbose = true) override;
private:
	const bool _defaultValue;
	bool& _value;
	void (UciManager::impl::*_callbackFunc)(bool);
	UciManager::impl* _uci;
};

class ButtonUciOption final: public UciOption
{
public:
	ButtonUciOption( const std::string& name, UciManager::impl* uci, void (UciManager::impl::*callbackFunc)());
	std::string print() const override;
	bool setValue( const std::string&, bool verbose = true ) override;
private:
	void (UciManager::impl::*_callbackFunc)();
	UciManager::impl* _uci;
};

#endif /* UCI_OPTION_H_ */

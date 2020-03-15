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

//---------------------------------------------
//	include
//---------------------------------------------

#include "uciOption.h"
#include "vajo_io.h"

/**********************************************************
UciOption
**********************************************************/
bool UciOption::operator==(const std::string& rhs)
{
	return _name == rhs;
}

UciOption::UciOption(const std::string& name):_name(name){}

/**********************************************************
StringUciOption
**********************************************************/

StringUciOption::StringUciOption( const std::string& name, std::string& value, UciManager::impl* uci, void (UciManager::impl::*callbackFunc)(std::string), const std::string& defVal):UciOption(name),_defaultValue(defVal),_value(value), _callbackFunc(callbackFunc), _uci(uci){ setValue(_defaultValue, false); }
std::string StringUciOption::print() const
{
	std::string s = "option name ";
	s += _name;
	s += " type string default ";
	s += _defaultValue;
	return s;
}

bool StringUciOption::setValue( const std::string& s, bool verbose)
{
	_value = s;
	if(_callbackFunc)
	{
		(_uci->*_callbackFunc)(_value);
	}
	if(verbose)
	{
		sync_cout<<"info string "<<_name<<" set to "<<_value<<sync_endl;
	}
	return true;
}

/**********************************************************
SpinUciOption
**********************************************************/
SpinUciOption::SpinUciOption( const std::string& name, unsigned int& value, UciManager::impl* uci, void (UciManager::impl::*callbackFunc)(unsigned int), const unsigned int defVal, const unsigned int minVal, const int unsigned maxVal):
	UciOption(name),
	_defValue(defVal),
	_minValue(minVal),
	_maxValue(maxVal),
	_value(value),
	_callbackFunc(callbackFunc),
	_uci(uci)
{ 
	setValue( std::to_string(_defValue), false );
}

std::string SpinUciOption::print() const {
	std::string s = "option name ";
	s += _name;
	s += " type spin default ";
	s += std::to_string(_defValue);
	s += " min ";
	s += std::to_string(_minValue);
	s += " max ";
	s += std::to_string(_maxValue);
	return s;
}

bool SpinUciOption::setValue( const std::string& s, bool verbose)
{
	int value;
	try
	{
		value = std::stoi(s);
	}
	catch(...)
	{
		return false;
	}
	if( value < (int)_minValue ) { value = _minValue; }
	if( value > (int)_maxValue ) { value = _maxValue; }
	
	_value = value;
	
	if( _callbackFunc )
	{
		(_uci->*_callbackFunc)(_value);
	}
	if(verbose)
	{
		sync_cout<<"info string "<<_name<<" set to "<<_value<<sync_endl;
	}
	return true;
}		

/**********************************************************
CheckUciOption
**********************************************************/

CheckUciOption::CheckUciOption( const std::string& name, bool& value, const bool defVal):UciOption(name),_defaultValue(defVal), _value(value)
{
	setValue( _defaultValue ? "true" : "false", false );
}
std::string CheckUciOption::print() const {
	std::string s = "option name ";
	s += _name;
	s += " type check default ";
	s += ( _defaultValue ? "true" : "false" );
	return s;
}

bool CheckUciOption::setValue( const std::string& s, bool verbose)
{
	if( s == "true" )
	{
		_value = true;
		if(verbose)
		{
			sync_cout<<"info string "<<_name<<" set to "<<s<<sync_endl;
		}
		
	}
	else if( s == "false" )
	{
		_value = false;
		if(verbose)
		{
			sync_cout<<"info string "<<_name<<" set to "<<s<<sync_endl;
		}
	}
	else
	{
		sync_cout<<"info string error setting "<<_name<<sync_endl;
		return false;
	}
	return true;
}


/**********************************************************
ButtonUciOption
**********************************************************/
ButtonUciOption::ButtonUciOption( const std::string& name, UciManager::impl* uci, void (UciManager::impl::*callbackFunc)()):UciOption(name), _callbackFunc(callbackFunc), _uci(uci){}

std::string ButtonUciOption::print() const {
	std::string s = "option name ";
	s += _name;
	s += " type button";
	return s;
}

bool ButtonUciOption::setValue( const std::string&, bool)
{
	if(_callbackFunc)
	{
		(_uci->*_callbackFunc)();
	}
	return true;
}


/********************************************************************************************************

Authors:		(c) 2023 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
********************************************************************************************************

Description:

********************************************************************************************************/
#include "util.h"

#include <stdexcept>
#include <sstream>
#include <fstream>


/*******************************************************************************************************

*******************************************************************************************************/

void ThrowNullPointer(const std::string& file, const std::string& function, int line) {
	const auto n = file.find_last_of("/\\");

	std::string shortFile{};
	if (n == std::string::npos) shortFile = file; else shortFile = file.substr(n + 1);

	std::stringstream ss{};
	ss << "A null pointer was passed to a function.\n\nFunciton: " << function << "\nFile: " << shortFile << "\nline: " << line;
	throw(std::invalid_argument(ss.str().c_str()));
}

/*******************************************************************************************************

*******************************************************************************************************/
void ThrowPrettyException(const std::string& file, const std::string& function, int line, const std::string& message) {
	const auto n = file.find_last_of("/\\");

	std::string shortFile{};
	if (n == std::string::npos) shortFile = file; else shortFile = file.substr(n + 1);

	std::stringstream ss{};
	ss << message << "\n\nFunciton: " << function << "\nFile: " << shortFile << "\nline: " << line;
	throw(std::exception(ss.str().c_str()));
}


/*******************************************************************************************************
Writes a log entry if in debug mode
*******************************************************************************************************/
#ifdef _DEBUG
void dev_log(std::string s) {

	std::ofstream file("c:\\temp\\ofxlog.txt", std::ios::app);
	file << " : " << s << std::endl;

	return;
}
#else
void dev_log(std::string) {}
#endif



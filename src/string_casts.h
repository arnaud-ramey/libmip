/*!
  \file        string_casts.h
  \author      Arnaud Ramey <arnaud.a.ramey@gmail.com>
                -- Robotics Lab, University Carlos III of Madrid
  \date        2013/12/31

________________________________________________________________________________

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
________________________________________________________________________________

\todo Description of the file

 */

#ifndef STRING_CASTS_H
#define STRING_CASTS_H

#include <string>
#include <sstream>

namespace StringUtils {

/*! cast a string to another type
 * \param in the string to cast
 * \param success true if we managed, false otherwise
 * \return the cast
 */
template<class _T>
_T cast_from_string(const std::string & in, bool & success) {
  std::istringstream myStream(in);
  _T ans;
  success = (myStream >> ans);
  return ans;
}
/*! cast a string to another type
 * \param in the string to cast
 * \return the cast
 */
template<class _T>
_T cast_from_string(const std::string & in) {
  bool success;
  return cast_from_string<_T> (in, success);
}

/*! cast any type to string
 * \param in the value to cast
 * \param success true if we managed, false otherwise
 * \return the cast
 */
template<class _T>
std::string cast_to_string(const _T in, bool & success) {
  std::ostringstream ans;
  success = (ans << in);
  return ans.str();
}

/*! cast any type to string
 * \param in the value to cast
 * \return the cast
 */
template<class _T>
std::string cast_to_string(const _T in) {
  bool success;
  return cast_to_string<_T> (in, success);
}

/*! cast a type to an explicit string
  \example string -> 'string' */
template<class _T>
inline std::string cast_type_to_string();


////////////////////// template specializations ////////////////////////////////

/*! a specialization for cast_from_string() */
template<> inline
const char* cast_from_string(const std::string & in, bool & success) {
  success = true;
  return in.c_str();
}

template<> inline
std::string cast_from_string(const std::string & in, bool & success) {
  success = true;
  return in;
}

/*! a specialization for cast_type_to_string() */
template<> inline std::string cast_type_to_string<bool>() {
  return "bool";
}
/*! a specialization for cast_type_to_string() */
template<> inline std::string cast_type_to_string<char>() {
  return "char";
}
/*! a specialization for cast_type_to_string() */
template<> inline std::string cast_type_to_string<short>() {
  return "short";
}
/*! a specialization for cast_type_to_string() */
template<> inline std::string cast_type_to_string<int>() {
  return "int";
}
/*! a specialization for cast_type_to_string() */
template<> inline std::string cast_type_to_string<long int>() {
  return "long int";
}
/*! a specialization for cast_type_to_string() */
template<> inline std::string cast_type_to_string<float>() {
  return "float";
}
/*! a specialization for cast_type_to_string() */
template<> inline std::string cast_type_to_string<double>() {
  return "double";
}
/*! a specialization for cast_type_to_string() */
template<> inline std::string cast_type_to_string<std::string>() {
  return "std::string";
}
/*! a specialization for cast_type_to_string() */
template<> inline std::string cast_type_to_string<const char*>() {
  return "const char*";
}

} // end namespace StringUtils

#endif // STRING_CASTS_H

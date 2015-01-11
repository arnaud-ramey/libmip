/*!
  \file        string_split.h
  \author      Arnaud Ramey <arnaud.a.ramey@gmail.com>
                -- Robotics Lab, University Carlos III of Madrid
  \date        2014/1/2

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

#ifndef STRING_SPLIT_H
#define STRING_SPLIT_H

#include "string_casts.h"
#include <vector>

namespace StringUtils {

/*! \brief   split a string
 * \param   str the long string
 * \param   delim the string which is the separator between words, for instance '/'
 * \param   results a pointer towards a vector of string, will contain the results
 */
inline void StringSplit(const std::string & str, const std::string & delim,
                        std::vector<std::string>* results) {
  // maggieDebug3("StringSplit(str:'%s', delim:'%s')", str.c_str(), delim.c_str());
  results->clear();
  if (str == "")
    return;
  size_t delim_pos, search_pos = 0;
  while (search_pos <= str.size() - 1) {
    delim_pos = str.find(delim, search_pos);
    //maggieDebug1("delim_pos:%i, search_pos:%i", delim_pos, search_pos);
    if (delim_pos == std::string::npos) { // no more delim
      results->push_back(str.substr(search_pos));
      return;
    }
    if (delim_pos > 0) // == 0 only happens if str starts with delim
      results->push_back(str.substr(search_pos, delim_pos - search_pos));
    search_pos = delim_pos + delim.size();
    // quit if we reached the end of the std::string or std::string empty
  }
} // end StringSplit();

////////////////////////////////////////////////////////////////////////////////

template<class _T>
void StringSplit_(const std::string & str, const std::string & delim,
                  std::vector<_T>* results) {
  // maggieDebug3("StringSplit_(str:'%s', delim:'%s')", str.c_str(), delim.c_str());
  bool success;
  results->clear();
  if (str == "")
    return;
  size_t delim_pos, search_pos = 0;
  while (search_pos <= str.size() - 1) {
    delim_pos = str.find(delim, search_pos);
    //maggieDebug1("delim_pos:%i, search_pos:%i", delim_pos, search_pos);
    if (delim_pos == std::string::npos) { // no more delim
      results->push_back(cast_from_string<_T>(str.substr(search_pos),
                                              success));
      return;
    }
    if (delim_pos > 0) // == 0 only happens if str starts with delim
      results->push_back(cast_from_string<_T>(str.substr(search_pos, delim_pos - search_pos),
                                              success));
    search_pos = delim_pos + delim.size();
    // quit if we reached the end of the string or string empty
  }
} //end StringSplit_()
} // end namespace StringUtils

#endif // STRING_SPLIT_H

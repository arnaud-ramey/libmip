/*!
  \file        exec_system_get_output.h
  \author      Arnaud Ramey <arnaud.a.ramey@gmail.com>
                -- Robotics Lab, University Carlos III of Madrid
  \date        2014/11/15

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
#ifndef EXEC_SYSTEM_GET_OUTPUT_H
#define EXEC_SYSTEM_GET_OUTPUT_H

#include <stdio.h>
#include <string>

/*!
 * Execute a command and get its output.
 * Careful though, be aware that this will only grab stdout and not stderr
 * From http://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c
 * \param cmd
 * \return
 *    the output of that command
 */
inline std::string exec_system_get_output(const char* cmd) {
  // printf("exec_system_get_output('%s')\n", cmd);
  FILE* pipe = popen(cmd, "r");
  if (!pipe) {
    printf("exec_system_get_output('%s'): could not open pipe\n", cmd);
    return "ERROR";
  }
  char buffer[128];
  std::string result = "";
  while(!feof(pipe)) {
    if(fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  pclose(pipe);
  return result;
}

#endif // EXEC_SYSTEM_GET_OUTPUT_H

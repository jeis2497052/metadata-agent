/*
 * Copyright 2017 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

#include "time.h"

#include <cstdlib>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <time.h>

namespace google {

namespace rfc3339 {

namespace {

// See http://stackoverflow.com/questions/4137748.
int UTCOffset() {
  const std::time_t local = std::time(nullptr);
  const std::time_t utc = std::mktime(std::gmtime(&local));
  return utc - local;
}

const int kUtcOffset = UTCOffset();

}

std::string ToString(const std::chrono::system_clock::time_point& t) {
  std::stringstream out;
  const std::time_t time = std::chrono::system_clock::to_time_t(t);
  std::tm utc_time = safe_gmtime(&time);
  // GCC 4.x does not implement std::put_time. Sigh.
  char dt[64];
  std::strftime(dt, sizeof(dt), "%Y-%m-%dT%H:%M:%S", &utc_time);
  std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>
      t_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(t);
  std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
      t_s = std::chrono::time_point_cast<std::chrono::seconds>(t_ns);
  const std::chrono::nanoseconds ns = t_ns - t_s;
  out << dt << "." << std::setw(9) << std::setfill('0') << ns.count() << "Z";
  return out.str();
}

std::chrono::system_clock::time_point FromString(const std::string& s) {
  std::tm tm;
  const char* end = strptime(s.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
  if (end == nullptr || end - s.c_str() != s.find('.')) {
    // TODO
    return std::chrono::system_clock::time_point();
  }
  char* zone;
  long ns = std::strtol(end + 1, &zone, 10);
  if (zone <= end + 1 || *zone != 'Z' || *(zone+1) != '\0') {
    // TODO
    return std::chrono::system_clock::time_point();
  }
  const std::time_t local_time = std::mktime(&tm);
  const std::time_t utc_time = local_time + kUtcOffset;
  std::chrono::system_clock::time_point sec =
      std::chrono::system_clock::from_time_t(utc_time);
  return std::chrono::time_point_cast<std::chrono::system_clock::duration>(
      sec + std::chrono::nanoseconds(ns));
}

}

namespace {

std::mutex localtime_mutex;
std::mutex gmtime_mutex;

}

std::tm safe_localtime(const std::time_t* t) {
  std::lock_guard<std::mutex> l(localtime_mutex);
  return *std::localtime(t);
}

std::tm safe_gmtime(const std::time_t* t) {
  std::lock_guard<std::mutex> l(gmtime_mutex);
  return *std::gmtime(t);
}

}

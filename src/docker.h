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
#ifndef DOCKER_H_
#define DOCKER_H_

//#include "config.h"

#include <vector>

#include "configuration.h"
#include "environment.h"
#include "updater.h"

namespace google {

class DockerReader {
 public:
  DockerReader(const MetadataAgentConfiguration& config);
  // A Docker metadata query function.
  std::vector<MetadataUpdater::ResourceMetadata> MetadataQuery() const;

 private:
  const MetadataAgentConfiguration& config_;
  Environment environment_;
};

class DockerUpdater : public PollingMetadataUpdater {
 public:
  DockerUpdater(MetadataAgent* server)
      : reader_(server->config()), PollingMetadataUpdater(
          server, server->config().DockerUpdaterIntervalSeconds(),
          std::bind(&google::DockerReader::MetadataQuery, &reader_)) { }
 private:
  DockerReader reader_;
};

}

#endif  // DOCKER_H_

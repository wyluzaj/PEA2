#ifndef PEA2_WRITER_H
#define PEA2_WRITER_H
#pragma once

#include <string>
#include "tsp_instance.h"

std::string sanitizeFileName(const std::string& input);

void writeBnBSummaryToFile(TSPResult& result);
void appendBnBResultToCsv(const TSPResult& result,
                          const std::string& csvPath = "results/summary_bnb.csv");
void printBnBResultToConsole(const TSPResult& result);
#endif //PEA2_WRITER_H

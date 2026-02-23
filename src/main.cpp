/*
ArchLLM-Lab
-----------
Objective:
Maximize retained semantic information under strict token budget.

Formalized as:
Max sum(v_i * x_i)
Subject to sum(t_i * x_i) <= B

Where:
v_i = information value
t_i = token cost
B   = context window constraint
*/

struct Message {
    std::string content;
    int tokens;
    double importance;
    double density;
};

double computeImportance(const std::string& text, int recencyIndex) {
    double entropyProxy = std::log(text.length() + 1);
    double recencyWeight = std::exp(-0.15 * recencyIndex);
    return entropyProxy * recencyWeight;
}
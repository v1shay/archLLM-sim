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

std::vector<Message> compressContext(
    std::vector<std::string> messages,
    int tokenBudget
) {
    std::vector<Message> scored;

    for (int i = 0; i < messages.size(); ++i) {
        int tokenEstimate = messages[i].length() / 4;
        double importance = computeImportance(messages[i], i);
        double density = importance / std::max(tokenEstimate, 1);

        scored.push_back({messages[i], tokenEstimate, importance, density});
    }

    std::sort(scored.begin(), scored.end(),
        [](const Message& a, const Message& b) {
            return a.density > b.density;
        });

    std::vector<Message> selected;
    int usedTokens = 0;

    for (auto& msg : scored) {
        if (usedTokens + msg.tokens <= tokenBudget) {
            selected.push_back(msg);
            usedTokens += msg.tokens;
        }
    }

    return selected;
}
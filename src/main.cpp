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

int main() {
    std::vector<std::string> conversation = {
        "User wants help designing LLM memory compression.",
        "Discussion about entropy-based scoring methods.",
        "Irrelevant tangent about weather conditions.",
        "Important constraint: preserve key system architecture decisions.",
        "Token limit is strictly enforced at 100 tokens."
    };

    int tokenBudget = 60;

    auto compressed = compressContext(conversation, tokenBudget);

    std::cout << "=== Selected Context ===\n";
    for (auto& msg : compressed) {
        std::cout << "- " << msg.content << "\n";
    }

    return 0;
}

double redundancyPenalty(const std::string& a, const std::string& b) {
    if (a == b) return 1.0;
    return 0.0; // placeholder — extend to cosine similarity later
}
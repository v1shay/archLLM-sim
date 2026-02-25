#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

struct Message {
    std::string content;
    int tokens;
    double importance;
    double density;
};

// --- SCORING ENGINE ---

double computeImportance(const std::string& text, int distanceFromNow) {
    // text.length() is a proxy for raw information content
    double entropyProxy = std::log(text.length() + 1.0);
    
    // FIX: Exponential decay should apply to DISTANCE from now.
    // distance 0 (newest) -> weight 1.0
    // distance 5 (oldest) -> weight ~0.47
    double recencyWeight = std::exp(-0.15 * distanceFromNow); 
    
    return entropyProxy * recencyWeight;
}

double redundancyPenalty(const std::string& a, const std::string& b) {
    // placeholder — extend to cosine similarity or Levenshtein later
    if (a == b) return 1.0; 
    return 0.0; 
}

// --- CORE LOGIC ---

std::vector<Message> compressContext(std::vector<std::string> messages, int tokenBudget) {
    std::vector<Message> scored;
    int n = messages.size();

    for (int i = 0; i < n; ++i) {
        // Token estimate (crude 4 char/token rule)
        int tokenEstimate = std::max((int)messages[i].length() / 4, 1);
        
        // FIX: Calculate distance from the end of the conversation
        int distanceFromNow = (n - 1) - i; 
        
        double importance = computeImportance(messages[i], distanceFromNow);
        double density = importance / tokenEstimate;

        scored.push_back({messages[i], tokenEstimate, importance, density});
    }

    // Sort by density: Highest Value per Token first (Greedy Knapsack)
    std::sort(scored.begin(), scored.end(), [](const Message& a, const Message& b) {
        return a.density > b.density;
    });

    std::vector<Message> selected;
    int usedTokens = 0;

    for (const auto& msg : scored) {
        if (usedTokens + msg.tokens <= tokenBudget) {
            selected.push_back(msg);
            usedTokens += msg.tokens;
        }
    }

    return selected;
}

double computeRetentionScore(const std::vector<Message>& selected) {
    double score = 0;
    for (const auto& msg : selected)
        score += msg.importance;
    return score;
}

// --- EXECUTION ---

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

    std::cout << "=== Selected Context (Budget: " << tokenBudget << " tokens) ===\n";
    for (const auto& msg : compressed) {
        std::cout << "• [Value: " << (int)(msg.importance * 10) / 10.0 
                  << " | Tokens: " << msg.tokens << "] " << msg.content << "\n";
    }

    std::cout << "\nTotal Semantic Retention Score: " << computeRetentionScore(compressed) << "\n";

    return 0;
}

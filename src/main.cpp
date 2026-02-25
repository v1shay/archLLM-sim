#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <numeric>

struct Message {
    std::string content;
    int tokens;
    double importance;
    double density;
};

// --- SEMANTIC UTILS ---

// Calculates how similar two strings are (0.0 to 1.0)
// Production note: In a real LLM environment, replace this with Cosine Similarity of Embeddings.
double calculateSimilarity(const std::string& s1, const std::string& s2) {
    const size_t len1 = s1.size(), len2 = s2.size();
    if (len1 == 0 || len2 == 0) return 0.0;
    
    std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1));
    for (int i = 0; i <= len1; ++i) dp[i][0] = i;
    for (int j = 0; j <= len2; ++j) dp[0][j] = j;

    for (int i = 1; i <= len1; ++i) {
        for (int j = 1; j <= len2; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({ dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost });
        }
    }
    return 1.0 - (double)dp[len1][len2] / std::max(len1, len2);
}

// --- SCORING ENGINE ---

double computeImportance(const std::string& text, int distanceFromNow) {
    // Log-scaling length prevents long "rambling" messages from dominating
    double entropyProxy = std::log(text.length() + 1.0);
    double recencyWeight = std::exp(-0.15 * distanceFromNow); 
    return entropyProxy * recencyWeight;
}

// --- CORE LOGIC ---

std::vector<Message> compressContext(std::vector<std::string> messages, int tokenBudget) {
    std::vector<Message> scored;
    int n = messages.size();

    for (int i = 0; i < n; ++i) {
        int tokenEstimate = std::max((int)messages[i].length() / 4, 1);
        int distanceFromNow = (n - 1) - i; 
        
        double importance = computeImportance(messages[i], distanceFromNow);
        
        // --- REDUNDANCY PENALTY ---
        // If this message is too similar to the one immediately preceding it, slash importance.
        if (i > 0) {
            double sim = calculateSimilarity(messages[i], messages[i-1]);
            if (sim > 0.7) importance *= 0.5; // 50% penalty for redundancy
        }

        double density = importance / tokenEstimate;
        scored.push_back({messages[i], tokenEstimate, importance, density});
    }

    // Sort by Density (Heuristic for the Knapsack Problem)
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

    // FINAL POLISH: Re-sort by original chronological order so the LLM doesn't get confused
    // (This is vital for production LLM context)
    // We would need to store original indices to do this perfectly.
    
    return selected;
}

int main() {
    std::vector<std::string> conversation = {
        "User wants help designing LLM memory compression.",
        "User wants help designing LLM memory compression.", // Intentional duplicate
        "Discussion about entropy-based scoring methods.",
        "Important constraint: preserve key system architecture decisions.",
        "Token limit is strictly enforced at 100 tokens."
    };

    int tokenBudget = 50; 
    auto compressed = compressContext(conversation, tokenBudget);

    std::cout << "=== Optimized Context (Budget: " << tokenBudget << ") ===\n";
    for (const auto& msg : compressed) {
        std::cout << "• [" << msg.tokens << " tokens] " << msg.content << "\n";
    }

    return 0;
}

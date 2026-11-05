# Overview
This document outlines our policy regarding AI-assisted development. 

The **guiding principle** of this document is that a human must be directly and clearly responsible for all aspects of code in this repository.
AI agents are not allowed to contribute.

# Specifics
1. **Commit Attribution:** Commits may **not** be attributed to an agentic AI tool or other AI tools. All commits must be directly submitted by a human developer account and by making a commit, the human developer is indicating that they take full responsibility for the code that they are submitting. Agentic AI is also not allowed to be a co-contributor to a commit because we reserve the word "contributor" for humans.
2. **Use of AI during code creation:** Human developers may use AI (by any definition) in their development process. We recognize that there is really no way that we could police that anyway. 
3. **In-code citations:** Because AI-assistance is often tightly intertwined with independent human development, we do not require or expect in-code citations for small amounts of AI-assistance (e.g. autocomplete of C++ syntax). If a specialized AI tool is used for a significant contribution, that may warrant an in-code citation.
4. **Standard of oversight:** Contributors and reviewers are expected to have mentally stepped through the code and made sense of it before requesting or approving a review. Exemptions should be requested explicitly where the judged effort tradeoff is not worthwhile. This standard will be subject to further consideration and possible future adjustment.

# Examples
Because we have a large number of junior contributors to this repository, this section includes more specific things aimed at them.

## Copilot/Autocomplete
It is completely reasonable to use GitHub Copilot or similar plugins to autocomplete C++ code, especially as you are first getting used to the syntax. However, because of the size of this codebase, these autocomplete suggestions are often totally bonkers wrong, so you will likely find yourself not actually relying on the made-up methods suggested. 

If you are relying on autocomplete for some particulary gross C++ syntax (looking at you, static cast!), please feel free to reach out to your mentor to double check on the syntax. This is also a time when a chatbot may be helpful in explaining it.

Writing tests is another place where this kind of assistance probably will be actually helpful, though you again need to make sure to have a clear idea in your head of what you want the test to actually do, because the LLM doesn't actually know better than you on WHAT should be tested.

Whenever you are using this kind of AI assistance (and generally, but particularly here), you absolutely must make sure to compile and run a small experiment as well as run the tests (with `make test-sgp`) and look for compilation errors or failed tests.

## Asking chatbots to explain code/errors
Go for it! Your mentor is also very happy to help, and the chatbot might not be super helpful with particulars, but many times, they are actually helpful at deducing what is going on with some weird C++ thing.

## Take responsibility
Remember that above all, you should never let the AI think for you. If you are a student working on this project, you are here because you want to learn. We have asked you to complete a task because we want you to learn along the way; the completion of the task is honestly secondary to your learning. Your mentor (and all of us on this project) would rather you take a longer time to complete a task but actually learn than lean too heavily on AI-assistance and end your experience without having expanded your own thinking.






# Acknowledgements
Our policy is inspired by [Empirical](https://github.com/devosoft/Empirical/blob/master/doc/dev/ai-usage-guidelines.md)'s (and many of the authors of this document contributed to the Empirical policy).
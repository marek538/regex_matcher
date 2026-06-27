# Regular Expression Matcher (Abstract Syntax Tree to Nondeterministic Finite Automaton)

## Project Description
A custom regular expression evaluation engine that determines which words from a given dataset belong to the language defined by a specific regex. The regex is evaluated from a given Abstract Syntax Tree (AST) supporting union, concatenation, iteration, symbols, epsilon, and empty sets.

## Solution & Algorithm
The algorithm converts the Regex AST into a Non-deterministic Finite Automaton (NFA) using a bottom-up approach based on **Glushkov's Construction (Position Automaton)**.
* **Automaton Construction:** Recursively calculates initial states (`beginnings`), final states (`ends`), and state transitions (`neighbors`) for each AST node.
* **Simulation:** Simulates the constructed NFA over the input words by maintaining and updating a set of active states, effectively matching the string on the fly.

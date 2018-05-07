# sr-lib
C++ Automatic Speech Recognition library for my [BTech Project](https://github.com/theawless/BTech-Project). 

### Features
* Isolated Word Recognition
* Word Prediction
* Sentence Recognition (using isolated words)
* Parallel and fast
* Configurable
* Portable

### Results
* Test recordings are english digts (words) and few country codes (sentences)
* Accuracy for single speaker isolated word recognition: ~90%

### Methods
* Audio Preprocessor
* Linear Predictive Cepstral Coefficients (LPCC)
* Mel Filter Cepstral Coefficients (MFCC)
* Derivative Cepstral Coefficients (delta, accel)
* Vector Quantisation (VQ) - LBG, KMeans
* Hidden Markov Model (HMM) - Baum–Welch, Viterbi
* DFA and NGram

### Extras
* CSV Reader
* Wave Reader
* Logger
* Thread Pool
* Config Parser

### How to build?

sr-lib doesn't depend on any libraries other than the standard ones.

For windows:
* Get Visual Studio 2017 (with cmake tools).
* Open sr-lib as a folder
* Click `Generate cache` from CMake menu
* Click `Build All` from CMake menu
* Select run target WordRecognition (preferably x64 release)
* Run to see the demo
* Profit???

For others:
* Use the command line :(

### How to improve accuracy?

In general, isolated word recognition works well for:

* distinct words in the vocabulary
* short utterance durations
* fewer words in the vocabulary

These parameters can be fine tuned:

| key            | type    | description                                                 |
| ---------------| ------- | ---------------------------------------------------------   |
| q_cache        | bool    | whether cached training files should be used                |
| n_thread       | int     | number of threads used for parallel execution               |
| q_trim         | bool    | whether the samples should be trimmed for background noise  |
| x_frame        | int     | number of samples in a frame                                |
| x_overlap      | int     | number of samples to be overlapped while framing            |
| cepstra        | string  | "mfc" or "lpc" variants of feature generation               |
| n_cepstra      | int     | number of features                                          |
| q_gain         | bool    | whether gain term should be added to features               |
| q_delta        | bool    | whether delta terms should be added to features             |
| q_accel        | bool    | whether accel terms should be added to features             |
| x_codebook     | int     | size of codebook                                            |
| n_state        | int     | number of states in HMM                                     |
| n_bakis        | int     | connentedness of initial bakis model for HMM                |
| n_retrain      | int     | number of times each model should be trained                |
| n_gram         | int     | number of previous words to be considered for prediction    |
| q_dfa          | bool    | command based word prediction or probability based          |
| gram_weight    | double  | linear weight for the final scoring with recognition result |
| cutoff_score   | double  | cutoff for final score                                      |

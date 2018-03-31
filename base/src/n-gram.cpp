#include "n-gram.h"

#include <sstream>

#include "utils.h"

using namespace std;

bool Gram::empty() const
{
	return counts.empty();
}

istream &operator>>(istream &input, Gram &gram)
{
	vector<vector<string>> mat = Utils::get_matrix_from_stream<string>(input, '=');

	for (int i = 0; i < mat.size(); ++i)
	{
		stringstream stream(mat[i][0]);
		gram.counts[Utils::get_vector_from_stream<string>(stream, '|')] = stoi(mat[i][1]);
	}

	return input;
}

ostream &operator<<(ostream &output, const Gram &gram)
{
	vector<vector<string>> mat;

	for (map<vector<string>, int>::const_iterator it = gram.counts.begin(); it != gram.counts.end(); ++it)
	{
		vector<string> vec{ Utils::get_string_from_vector<string>(it->first, '|'), to_string(it->second) };
		mat.push_back(vec);
	}
	output << Utils::get_string_from_matrix<string>(mat, '=');

	return output;
}

MLE::MLE(vector<Gram> grams) :
	grams(grams)
{
}

double MLE::score(const vector<string> &context, const string &word) const
{
	double P = 0.0;

	vector<string> sentence = context; sentence.push_back(word);
	if (grams[sentence.size()].counts.find(sentence) != grams[sentence.size()].counts.end())
	{
		P = (double)grams[sentence.size()].counts.at(sentence) / grams[context.size()].counts.at(context);
	}

	return P;
}

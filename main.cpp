#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <limits>

// the structure representing a possible solution password as a member of a linked list containing all possible passwords
struct Guess 
{
	std::array<unsigned short, 4> code; // the password
	Guess* next; // address of the next password in the list
};

void initPegCombos(std::array<std::array<unsigned short, 2>, 15> combos);
std::string getColors(const Guess& g);
unsigned short simulateRedPegs(const Guess& g1, const Guess& g2);
unsigned short simulateWhitePegs(const Guess& g1, const Guess& g2);
void generatePasswords(std::vector<std::vector<unsigned short>>& passwords, const std::vector<unsigned short>& branch, unsigned short target_size);
std::vector<std::vector<unsigned short>> generatePasswords();
void prepareGuesses(Guess& head, const std::vector<std::vector<unsigned short>>& passwords);
void deleteGuess(Guess& predecessor);
void deduceByRedAndWhite(Guess& head, const Guess& guess, unsigned short red, unsigned short white);
const Guess& makeGuess(const Guess& head);
const Guess& maximinningGuess(const Guess& g);
int intInput(int minimum = INT_MIN, int maximum = INT_MAX, bool flush_after = false);
void flushI();

const std::array<std::string, 6> COLORS = {"red", "orange", "yellow", "green", "blue", "white"};
std::array<std::array<unsigned short, 2>, 15> PEG_COMBOS;

int main()
{
	std::cout << "Computronic Mastermind Solver\nprogrammed in late July, early August 2019 by R.H.Scriba" << std::endl <<
	"constant initial guess by Donald Knuth; guessing algorithm heavily inspired by Donald Knuth, from his 5-Step Algorithm." << std::endl << 
	std::endl;

	// generate all possible combinations of red and white response pegs
	initPegCombos(PEG_COMBOS);

	// create a dummy head node for the linked list of possible passwords
	Guess head = {{9,9,9,9}, nullptr};
	prepareGuesses(head, generatePasswords()); // generate all possible passowrds

	unsigned short red = 0, white = 0;

	unsigned short turn_no = 1;

	std::array<unsigned short, 4> ideal_first_colors = {0,0,1,1}; // Donald Knuth's optimal first guess
	Guess* ideal_first_guess = nullptr;

	// searching guesses for Knuth's ideal first guess
	for (Guess* i = head.next; i != nullptr && ideal_first_guess == nullptr; i = i->next)
		if (i->code == ideal_first_colors)
			ideal_first_guess = i;
	
	delete &ideal_first_colors;

	while (true)
	{
		// if it's the first guess, use Knuth's optimal first
		const Guess& guess = (turn_no == 1) ? *ideal_first_guess : maximinningGuess(head);

		std::cout << "Try: " << getColors(guess) << std::endl;

		std::cout << "How many red and how many white pegs? :  ";

		red = intInput(0);

		if (red == 4)
			break;

		white = intInput(0, INT_MAX, true);
		
		deduceByRedAndWhite(head, guess, red, white);

		++turn_no;
	}

	std::cout << "Done!";
}

// generate all possible combinations of red and white pegs that could be given as response to a guessed password
// combinations of red and white pegs are represented as an ordered pair: (# red, # white)
void initPegCombos(std::array<std::array<unsigned short, 2>, 15> combos)
{
	unsigned short i = 0; 
	for (unsigned short r = 0; r <= 4; ++r)
		for (unsigned short w = 0; w <= 4 - r; ++w)
			combos[i++] = {r, w};
}

// determine the number of red pegs that would be given as a response for a guess with a given possible solution
unsigned short simulateRedPegs(const Guess& guess, const Guess& possible_solution)
{
	unsigned short pegs = 0;

	for (unsigned short i = 0; i < 4; ++i)
		if (guess.code[i] == possible_solution.code[i])
			++pegs;

	return pegs;
}

// determine the number of white pegs that would be given as a response for a guess with a given possible solution
unsigned short simulateWhitePegs(const Guess& guess, const Guess& possible_solution)
{
	unsigned short pegs = 0;

	// once a color in the guessed password has been 'paired' with one in the same color but different position in the possible solution, it cannot
	// be counted again towards the white peg total, and must be excluded from further comparison. Similarly, if a pair of pegs warrant a red peg,
	// they also must be excluded from the white peg count.
	std::array<bool, 4> exclusions_g = {false, false, false, false}, // excluded pegs in the guess
		exclusions_ps = {false, false, false, false}; // excluded pegs in the possible solution

	for (unsigned short i = 0; i < 4; ++i) // excluding pegs which count toward the red peg total
		if (guess.code[i] == possible_solution.code[i])
			exclusions_g[i] = exclusions_ps[i] = true;

	for (unsigned short i = 0; i < 4; ++i)
		if (!(exclusions_g[i]))
			for (unsigned short j = 0; j < 4; ++j)
				// if a match has been found, increment white pegs and exclude those tokens from the passwords from future consideration
				if (!exclusions_ps[j] && guess.code[i] == possible_solution.code[j])
				{
					exclusions_g[i] = exclusions_ps[j] = true;
					++pegs;
					break;
				}

	return pegs;
}

// recursively generate all possible passwords. The `branch` argument represents a branching point in the tree of all possible passwords, from
// which more branches go to more brancing points or to passwords. The `passwords` arugment stores all the possible passwords.
void generatePasswords(std::vector<std::vector<unsigned short>>& passwords, const std::vector<unsigned short>& branch, unsigned short target_size)
{
	if (branch.size() >= target_size)
		passwords.push_back(branch);

	else
		for (unsigned short i = 0; i < 6; ++i)
		{
			std::vector<unsigned short> partial(branch);
			partial.push_back(i);
			generatePasswords(passwords, partial, target_size);
		}
}

// acts as a `front-end` for the recursive function that generates the passwords recursively. This method deals with the start conditions and
// initializes the vector to store the passwords
std::vector<std::vector<unsigned short>> generatePasswords() 
{

	std::vector<std::vector<unsigned short>> passes;
	std::vector<unsigned short> password;
	generatePasswords(passes, password, 4);

	return passes;
}

// constructs a linked list of passwords, called "guesses", with arrays instead of vectors storing the codes
void prepareGuesses(Guess& head, const std::vector<std::vector<unsigned short>>& passwords)
{
	Guess* i = &head;

	unsigned short pi = 0;
	for (std::vector<unsigned short> p : passwords)
	{
		Guess* temp = new Guess {{p[0], p[1], p[2], p[3]}, nullptr};

		i->next = temp;

		i = i->next;

		++pi;
	}

	std::cout << std::endl;
}

// delete a password from the linked list by accessing it's predecessor in the list
void deleteGuess(Guess& predecessor)
{
	Guess* deletee = predecessor.next;

	predecessor.next = deletee->next;

	delete deletee;
}

// remove passwords from the list of possibilities that couldn't be the solution based on the red and white pegs given for the previous guess
void deduceByRedAndWhite(Guess& possibilities, const Guess& guess, unsigned short red, unsigned short white)
{
	Guess* pre_i = &possibilities;
	
	while (pre_i->next != nullptr)
	{ // the guess must be passed first
		if (simulateRedPegs(guess, *(pre_i->next)) != red || simulateWhitePegs(guess, *(pre_i->next)) != white)
			deleteGuess(*pre_i);
	
		else
			pre_i = pre_i->next;
	}
}

// create a string representation of a password, mapping non-negative integers to colors
std::string getColors(const Guess& g)
{
	const std::array<unsigned short, 4>& code = g.code;
	
	std::string color_sequence = "";

	for (unsigned short c : code)
		color_sequence = color_sequence + " " + COLORS[c];

	return color_sequence;
}

// each password, for different possible responses of red and white pegs could cause the deduction of a number of passwords, known as its "hits".
// The number of hits varies per response. This method finds the possible password whose minimum hit count is greater than all others. Directly
// inspired by Donald Knuth's minimax approach, and knowingly suboptimal compared to it.
const Guess& maximinningGuess(const Guess& head)
{
	unsigned short maximin_hits = 0; // the number of hits for the password with minimum greater than all other passwords' minimum hits
	Guess* maximinning_guess = head.next; // the password with the above number of hits

	for (Guess* i = head.next; i != nullptr; i = i->next)
	{
		unsigned short min_hits = 1296; // 6 colors, 4 positions means 6^4 passwords
		for (const std::array<unsigned short, 2>& r : PEG_COMBOS) // iterate through all responses to find the minimum hit count
		{
			unsigned short hits = 0;
			for (Guess* j = head.next; j != nullptr; j = j->next)
				// if the current combination of red and white pegs is different than the number of simulated number of red and white pegs with
				// j as a possible solution and i as the guess, then j is one of i's "hits"
				if (simulateRedPegs(*i, *j) != r[0] || simulateWhitePegs(*i, *j) != r[1])
					++hits;

			if (hits < min_hits)
				min_hits = hits;
		}	

		if (min_hits > maximin_hits) // iterate through every password to find the maximum of the minimum hits counts
		{
			maximin_hits = min_hits;
			maximinning_guess = i;
		}
	}

	return *maximinning_guess;
}

// get integer input; retry if input is invalid
// copied from another personal project
int intInput(int minimum, int maximum, bool flush_after)
{
    while (true)
    {
        int i;
        std::cin >> i;

        if (std::cin.fail() || i < minimum || i > maximum)
        {
            std::cout << "Invalid input; retry: ";
            flushI();
        }

        else 
        {
            if (flush_after) flushI();
            return i;
        }
    }
}

// flush the input buffer
void flushI()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
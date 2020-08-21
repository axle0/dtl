#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <algorithm>
#include <tuple>
#include <math.h>
#include <chrono> 

using namespace std;

// Structures
struct sort_col {

	int index;

	sort_col(int index_inp) {
		this->index = index_inp;
	}

	bool operator() (const vector<string> &a, const vector<string> &b) {
		return (stof(a[index]) < stof(b[index]));
	}
};

void matrix_print(vector<vector<string>>);

struct node {
	// Parameters
	string attr;
	float splitval;
	node* left_node;
	node* right_node;
	string label;
	vector<vector<string>> data;

	// constructor
	node(string attr_, float splitval_, node* left_node_, node* right_node_, string label_, vector<vector<string>> data_) {
		attr = attr_;
		splitval = splitval_;
		left_node = left_node_;
		right_node = right_node_;
		label = label_;
		data = data_;
	}

	// Methods	
	void print_node() {
		cout << "{ Attr: " << attr << ", Splitval: " << splitval << ", Label: " << label << "}";
	}

	void print_children() {
		if(this->left_node != NULL) {
			cout << "[LEFT BRANCH] {" << this->attr << "} --> ";
			this->left_node->print_node();
		}

		if(this->right_node != NULL) {
			cout << "[RIGHT BRANCH] {" << this->attr << "} --> ";
			this->right_node->print_node();
		}
	}

	void print_data() {
		matrix_print(data);
	}
};


void print_tree(node* starting_node) {
	if(starting_node->left_node != NULL or starting_node->right_node != NULL) {
		cout << endl;
		starting_node->print_node();
		cout << ":\n";
		cout << "------------------ DATA ------------------" << endl << endl;
		matrix_print(starting_node->data);
		
		if(starting_node->left_node != NULL) {
			cout << "\tLEFT: ";
			starting_node->left_node->print_node();
			cout << endl;
		}

		if(starting_node->right_node != NULL) {
			cout << "\tRIGHT: ";
			starting_node->right_node->print_node();
			cout << endl;
		}

		if(starting_node->left_node != NULL)
			print_tree(starting_node->left_node);
		if(starting_node->right_node != NULL)
			print_tree(starting_node->right_node);
	}
}

vector<vector<string>> read_file(string);
void vector_print(vector<string>);
void int_vector_print(vector<int>);
void matrix_col_erase(vector<vector<string>>::iterator, vector<vector<string>>::iterator, int);
bool same_labels(vector<vector<string>>);
bool same_data(vector<vector<string>>);

// find the index of a key(element) within a vector. If non-existent, return -1
int find_vec_key_index(vector<string>, string);
// at what row to split the vector based on the 'splitval'
int attr_vector_split_index(vector<vector<string>>, int, float);
// slice the vector at mth and nth index. m <= n. inclusive
vector<vector<string>> slice_vector(vector<vector<string>>, int, int);
// count the labels in a dataset
vector<int> count_data_labels(vector<vector<string>>);
// Find the most frequent label. Break ties by returning first found max.
// If no data, return unknown
string most_freq_label(vector<vector<string>>);

// Find the information content of a set of data[attr]
float info_cont(vector<vector<string>> data);
// Find the remainder of a set of data split via a plitvalue along the attr_index'th attribute
float remainder(vector<vector<string>>, int, float);
// Find information gain
float info_gain(vector<vector<string>>, int, float);
// Find the best attribute and splitting value at a node
tuple<string, float> choose_split(vector<vector<string>>, vector<string>);

// Construct a Decision Tree
node* DTL(vector<vector<string>>, vector<string>, int);
// for each data entry, traverse the tree and make a decision (assign label)
void predict(node*, vector<vector<string>>, vector<string>);


int main(int argc, char** argv) {
	auto start_time = chrono::high_resolution_clock::now(); 

	// If 3 inputs not given while running the program, error and exit
	if(argc == 4) {
		// vars
		string train_dir = argv[1];
		string test_dir = argv[2];
		int minleaf = stoi(argv[3]);

		auto inp_vec = read_file(train_dir);
		auto test_vec = read_file(test_dir);

		// seperate the attribute list from the actual data
		auto inp_header = inp_vec[0];
		inp_vec.erase(inp_vec.begin());

		auto test_header = test_vec[0];
		test_vec.erase(test_vec.begin());

		// construct (train) the tree
		node* root_node = DTL(inp_vec, inp_header, minleaf);

		// test the tree
		predict(root_node, test_vec, test_header);


	} else {
		cout << "Program Requires 3 parameters to run. " << argc - 1 << " was entered!" << endl;
		return 0;
	}

	 auto stop_time = chrono::high_resolution_clock::now(); 
	 auto duration = chrono::duration_cast<chrono::seconds>(stop_time - start_time); 

	 cout << "Executed " << duration.count() << " s" << endl;
}


vector<vector<string>> read_file(string dir) {

	string str;							// cell
	char ch;							// cell element
	vector<string> row_vec;				// row
	vector< vector<string> > vec;		// row x col vector
	vector<string> header;				// attribute list

	// read train file
	ifstream file;
	file.open(dir);
	if (!file) {	// check for opening failrue
	    cout << "Unable to open the Training file. Specified dir was: " << dir << endl;
	    exit(0);
	}

	// input character by character
	while (file.get(ch)) {	

		// if whitespace, and a string exists, enter the cell element in the row
        if(ch == ' ') {
        	if(str != "")
        		row_vec.push_back(str);
        	str = "";

        // if a newline exists, enter the row in the main vector
        } else if(ch == '\n') {
        	if(str != "")				// if a string remains, there was no whiteline. add it!
        		row_vec.push_back(str);
        	str = "";
        	vec.push_back(row_vec);
        	row_vec.erase(row_vec.begin(), row_vec.end());

        // any other character is a cell composed of cell elements (characters)
        } else {
        	str += ch;
        }
    }

    // if on the end of the line a string remains without newline or whitespace, add the last cell and line(row)
    if(str != "") {
    	row_vec.push_back(str);
    	vec.push_back(row_vec);
    }

    return vec;
}


void matrix_print(vector< vector<string> > vec) {

	for(int r = 0; r < vec.size(); r++) {
		for(int c = 0; c < vec[r].size(); c++) {
			cout << vec[r][c] << " ";
		}
		cout << endl;
	}
}

void vector_print(vector<string> vec) {
	cout << "[";
	for(int i = 0; i < vec.size(); i++)
		if(i < vec.size() - 1)
			cout << vec[i] << " ";
		else
			cout << vec[i];
	cout << "]" << endl;
}

void int_vector_print(vector<int> vec) {
	cout << "[";
	for(int i = 0; i < vec.size(); i++)
		if(i < vec.size() - 1)
			cout << vec[i] << " ";
		else
			cout << vec[i];
	cout << "]" << endl;
}

void matrix_col_erase(vector<vector<string>>::iterator vec_start, vector<vector<string>>::iterator vec_end, int row_) {
	for(vector<vector<string>>::iterator i = vec_start; i != vec_end; i++)
		(*i).erase((*i).begin() + row_);
}

// at what row to split the vector based on the 'splitval'
int attr_vector_split_index(vector<vector<string>> vec, int attr_index, float splitval) {
	int i = 0;

	do{
		// cout << "\t" << vec[i][attr_index] << endl;
		if(stof(vec[i][attr_index]) > splitval) break;
	} while(++i < vec.size());
	return i-1;
}

// slice the vector at mth and nth index. m <= n. inclusive
vector<vector<string>> slice_vector(vector<vector<string>> vec, int m, int n) {
	vector<vector<string>> out_vec;
	for(int i = m; i <= n; i++)
		out_vec.push_back(vec[i]);
	return out_vec;
}

// find the index of a key(element) within a vector. If non-existent, return -1
int find_vec_key_index(vector<string> vec, string key) {
	vector<string>::iterator it = find(vec.begin(), vec.end(), key);
	if(it != vec.end())
		return distance(vec.begin(), it);
	return -1;
}

// count the labels in a dataset
vector<int> count_data_labels(vector<vector<string>> data) {
	int total_labels = data.size(), r = 0;
	vector<string> label_keeping_vec;
	vector<int> counter_vec;

	if(data.size() > 0) {
		for(int i = 0; i < total_labels; i++) {
			r = find_vec_key_index(label_keeping_vec, data[i][data[i].size() - 1]);
			// if the label already exists
			if(r > -1) {
				counter_vec[r] = counter_vec[r] + 1;

			// if the label doesn't exist
			} else {
				label_keeping_vec.push_back(data[i][data[i].size() - 1]);
				counter_vec.push_back(1);
			}
		}
	}

	return counter_vec;
}

// Find the information content of a set of data[attr]
float info_cont(vector<vector<string>> data) {
	vector<int> counter_vec;
	vector<string> label_keeping_vec;

	int total_labels = data.size();
	float info_cont = 0;

	/////////////////////////// count each label in the dataset ///////////////////////////
	int r = 0;

	if(data.size() > 0) {
		for(int i = 0; i < total_labels; i++) {
			r = find_vec_key_index(label_keeping_vec, data[i][data[i].size() - 1]);
			// if the label already exists
			if(r > -1) {
				counter_vec[r] = counter_vec[r] + 1;

			// if the label doesn't exist
			} else {
				label_keeping_vec.push_back(data[i][data[i].size() - 1]);
				counter_vec.push_back(1);
			}
		}
	}

	/////////////////////////// END count each label in the dataset ///////////////////////////

	// accumulatively calculate information content of the dataset
	for(int i = 0; i < counter_vec.size(); i++) {
		if((float)counter_vec[i]/total_labels > 0)	// This test should never fail
			info_cont += -(float)counter_vec[i]/total_labels * log2((float)counter_vec[i]/total_labels);
	}

	return info_cont;
}

// Find the remainder of a set of data split via a plitvalue along the attr_index'th attribute
float remainder(vector<vector<string>> data, int attr_index, float splitval) {
	int vector_slice_row_index;
	vector<vector<string>> left_data, right_data;
	float prob_left, prob_right;
	float info_cont_left, info_cont_right;

	// Find the two branches
	vector_slice_row_index = attr_vector_split_index(data, attr_index, splitval);
	left_data = slice_vector(data, 0, vector_slice_row_index);
	right_data = slice_vector(data, vector_slice_row_index + 1, data.size() - 1);

	// probabilities of the branches. Size identifies how many "possibilities"
	prob_left = (float)left_data.size() / (float)data.size();
	prob_right = (float)right_data.size() / (float)data.size();

	// Information content of the child nodes
	info_cont_left = info_cont(left_data);
	info_cont_right = info_cont(right_data);

	// Remainder
	return prob_left * info_cont_left + prob_right * info_cont_right;
}

// Find information gain
float info_gain(vector<vector<string>> data, int attr_index, float splitval) {
	float remainder_;
	float node_info_content;

	/////////////////////////// Info Content ///////////////////////////
	node_info_content = info_cont(data);
	/////////////////////////// END Info Content ///////////////////////////
	
	/////////////////////////// Remainder ///////////////////////////

	remainder_ = remainder(data, attr_index, splitval);
	
	/////////////////////////// END Remainder ///////////////////////////


	return node_info_content - remainder_;
}

// Find the best attribute and splitting value at a node. Return in that order as a tuple
tuple<string, float> choose_split(vector<vector<string>> data, vector<string> attrs) {
	float current_gain = 0, best_gain = 0;
	float splitval = 0;
	float best_split = 0; 
	string best_attr;
	float node_info_content;

	bool trace = false;

	// cout << std::setprecision(16);
	if(trace == true) {
		cout << "\n------------- CHOOSE SPLIT -------------" << endl << endl;
		vector_print(attrs);
		matrix_print(data);
		cout << endl << endl;
	}

	node_info_content = info_cont(data);

	// for each attribute. -1 to exclud results column
	for(int attr = 0; attr < attrs.size() - 1; attr++) {
		if(trace == true)
			cout << "Testing for " << attrs[attr] << ":" << endl;
		// sort along the attriute column
		sort(data.begin(), data.end(), sort_col(attr));
		// for each row - 1
		// cout << "in" << endl;

		for(int r = 0; r < data.size() - 1; r++) {
			splitval = 0.5*(stof(data[r][attr]) + stof(data[r+1][attr]));
			current_gain = node_info_content - remainder(data, attr, splitval);
			if(trace == true)
				cout << "\t{" << current_gain << ", " << splitval << "}" << endl;
			// If we maximise gain, reassign parametersfloat
			if(current_gain > best_gain) {
				if(trace == true)
					cout << "\t\tBest gain mod invoked. " << current_gain << " > " << best_gain << endl;
				best_gain = current_gain;
				best_split = splitval;
				best_attr = attrs[attr];
			}
		}
		// cout << "\t out" << endl;
	}

	if(trace == true) {
		cout << "\n\nCHOSE {" << best_gain << ", " << best_split << "} --> " << best_attr << endl;
		cout << "------------- END CHOOSE SPLIT -------------" << endl;
	}

	return make_tuple(best_attr, best_split);
}

bool same_labels(vector<vector<string>> data) {
	string base_label;
	if(data.size() > 0) {
		base_label = data[0][data[0].size() - 1];
		for(int i = 1; i < data.size(); i++) {
			if(data[i][data[i].size() - 1] != base_label) return false;
		}
		return true;
	}

	// If data is empty, same case as size < minleaf. So skips this test by returning true
	return true;
}

bool same_data(vector<vector<string>> data) {
	if(data.size() > 0) {
		// for every row - 1
		for(int r = 0; r < data.size() - 1; r++) {
			// for every column -1, excluding labels. No need for stof()
			for(int c = 0; c < data[r].size() - 1; c++) {
				if(data[r][c] != data[r+1][c]) return false;
			}
		}
		return true;
	}

	// If data is empty, same case as size < minleaf. So skips this test by returning true
	return true;
}

// Find the most frequent label. Break ties by returning first found max.
// If no data, return unknown
string most_freq_label(vector<vector<string>> data) {
	int total_labels = data.size(), r = 0;
	vector<string> label_keeping_vec;
	vector<int> counter_vec;
	int max_index = 0;

	// max comparison lambda function
	// auto max_comp = [](int a, int b) {
	// 	return (a < b);
	// };

	if(data.size() > 0) {
		// construct the counter and label_keeping vector
		for(int i = 0; i < total_labels; i++) {
			// if the label already exists
			r = find_vec_key_index(label_keeping_vec, data[i][data[i].size() - 1]);
			if(r > -1) {
				counter_vec[r] = counter_vec[r] + 1;
			// if the label doesn't exist
			} else {
				label_keeping_vec.push_back(data[i][data[i].size() - 1]);
				counter_vec.push_back(1);
			}
		}
	} else {
		// If it's empty, we don't know the label.
		return "unknown";
	}

	/* 
	find the maximum index from the counter vector and correlate it to the
	label_keeping vector to find the maximum label in the dataset

	return an unknown label if we find multiple instances of "max" value (no unique mode)
	*/
	// vector<int>::iterator it;
	// it = max_element(counter_vec.begin(), counter_vec.end(), max_comp);
	// max_index = distance(counter_vec.begin(), it);

	int prev_max = counter_vec[0];
	int max_counter = 1;
	for(int i = 1; i < counter_vec.size(); i++) {
		if(counter_vec[i] > prev_max) {
			prev_max = counter_vec[i];
			max_counter = 1;
			max_index = i;
		} else if(counter_vec[i] == prev_max)
			max_counter++;
	}

	if(max_counter > 1)
		return "unknown";

	return label_keeping_vec[max_index];
}

// Construct a Decision Tree
node* DTL(vector<vector<string>> data, vector<string> attrs, int minleaf) {
	/* 
			leaf node
		-----------------
	if we meet minleaf threshold
	if all the labels are the same
	if all the data point are the same
	if no more attributes left
	*/

	string label;
	node* n;

	// leafnode
	if(data.size() <= minleaf or same_labels(data) or same_data(data) or attrs.size() < 1) {
		label = most_freq_label(data);
		// vector_print(attrs);
		// matrix_print(data);
		// cout << "\n\tCHOSEN LABEL --> " << label << endl;
		n = new node("leaf", -1, NULL, NULL, label, data);

	// node
	} else {
		auto split_data = choose_split(data, attrs);											// split_data tuple
		string attr = get<0>(split_data);														// attribute name
		float splitval = get<1>(split_data);													// split value

		int attr_index = find_vec_key_index(attrs, attr);										// attribute index
		sort(data.begin(), data.end(), sort_col(attr_index));									// sort along chosen attr
		int split_index = attr_vector_split_index(data, attr_index, splitval);					// data splitting index

		vector<vector<string>> left_data = slice_vector(data, 0, split_index);					// left node data
		vector<vector<string>> right_data = slice_vector(data, split_index+1, data.size()-1);	// right node data
		

		n = new node(attr, splitval, NULL, NULL, "N/A", data);

		n->left_node = DTL(left_data, attrs, minleaf);
		n->right_node = DTL(right_data, attrs, minleaf);
	}

	return n;
}

// for each data entry, traverse the tree and make a decision (assign label)
void predict(node* n, vector<vector<string>> test_data, vector<string> test_header) {
	node* current_node = n;
	int attr_index;

	// test every row (every data entry)
	for(int i = 0; i < test_data.size(); i++) {
		// while n is not a leaf node
		while(current_node->left_node != NULL and current_node->right_node != NULL) {
			// find the index of the node's attribute in the attribute list of the test data
			attr_index = find_vec_key_index(test_header, current_node->attr);
			if(stof(test_data[i][attr_index]) <= current_node->splitval)
				current_node = current_node->left_node;
			else
				current_node = current_node->right_node;
		}
		// after finding a child node, print the label to std output
		cout << current_node->label << endl;
		// reset root node
		current_node = n;
	}
}
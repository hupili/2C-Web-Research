#include "pagerank.h"

string g_execute ;
string g_fn_link ;
string g_fn_escape ;
string g_fn_output ;
double g_alpha = 0.0 ;

typedef struct _node_t {
	int id ;
	double weight ;
} node_t ;

vector<list<node_t> > g_adj_list ;
vector<double> g_escape_vector ;
vector<double> g_probability ;

int g_node_count ;
//map<string, int> g_name2index ;
//map<int, string> g_index2name ;

int main(int argc, char **argv){
	if ( argc == 6 ){
		g_execute = argv[0] ;
		g_node_count = atoi(argv[1]) ;
		g_fn_link = argv[2] ;
		g_fn_escape = argv[3] ;
		g_fn_output = argv[4] ;
		sscanf(argv[5], "%lf", &g_alpha) ;
	} else {
		print_usage() ;
		exit(ERR_PARAMETER) ;
	}	

	fprintf(stderr, "start initialization\n") ;
	init() ;	
	fprintf(stderr, "finish initialization\n") ;

	vector<double> cur_probability ;
	cur_probability.reserve(g_node_count) ;
	for ( int i = 0 ; i < g_node_count ; i ++ ){
		cur_probability.push_back(0.0) ; // real construct elements 
	}
	int iteration_count = 1 ;
	double diff = 0.0 ;
	next_iteration(cur_probability) ;
	while ( (diff = average_diff(cur_probability, g_probability)) > eps ){
		fprintf(stderr, "iteration %d: %.10f\n", iteration_count, diff) ;
		g_probability = cur_probability ;
		if ( iteration_count >= MAX_ITERATION ){
			break ;
		}
		iteration_count ++ ;	
		next_iteration(cur_probability) ;
	}

	if ( iteration_count >= MAX_ITERATION ){
		fprintf(stderr, "iteration terminated: due to MAX_ITERATION=%d\n", MAX_ITERATION) ;
	} else {
		//fprintf(stderr, "iteration terminated: due to eps=%.10f\n", eps) ;
		fprintf(stderr, "iteration terminated: due to eps=%.10f\n", diff) ;
	}

	output() ;
	return 0 ;
}

void output(){
	FILE *fp_out = fopen(g_fn_output.c_str(), "w") ;
	if ( NULL == fp_out ){
		exit(ERR_FILE_OPEN) ;
	}

	for ( int i = 0 ; i < g_node_count ; i ++ ){
		//fprintf(fp_out, "%s\t%f\n", g_index2name[i].c_str(), g_probability[i]) ;	
		//fprintf(fp_out, "%s\t%g\n", g_index2name[i].c_str(), g_probability[i]) ;	
		fprintf(fp_out, "%d\t%g\n", i + 1, g_probability[i]) ;	
	}
	fclose(fp_out) ;
}

void next_iteration(vector<double> & p){
	double escape = 0.0 ;
	for (vector<double>::iterator it = p.begin() ;
			it != p.end() ; it ++ ){
		*it = 0.0 ;
	}
	
	//transfer portion
	for (int from = 0 ; from < g_node_count ; from ++ ){
		//double transfer = g_alpha * g_probability[from] / g_adj_list[from].size() ;
		double transfer = g_alpha * g_probability[from] ;
		if ( g_adj_list[from].size() == 0 ) { 
			escape += g_probability[from] ;
		} else {
			escape += g_probability[from] * (1 - g_alpha) ;
		}
		list<node_t> &l = g_adj_list[from] ;
		for ( list<node_t>::iterator it = l.begin() ;
				it != l.end () ; it ++ ){
			p[it->id] += transfer * it->weight ;
		}
	}

	//escape portion
	for (int i = 0 ; i < g_node_count ; i ++ ){
		p[i] += escape * g_escape_vector[i] ;
	}
}

double average_diff(vector<double> &p1, vector<double> &p2){
	//double diff = 0.0 ;
	//for (int i = 0 ; i < g_node_count ; i ++ ){
	//	diff += fabs(p1[i] - p2[i]) ;
	//}
	//return diff / g_node_count ;
	
	//sara difference:
	//a norm vector dot product it self should be 1
	double norm_p1 = 0.0 ;
	double norm_p2 = 0.0 ;
	double prod = 0.0 ;
	for ( int i = 0 ; i < g_node_count ; i ++ ){
		norm_p1 += p1[i] * p1[i] ;
		norm_p2 += p2[i] * p2[i] ;
		prod += p1[i] * p2[i] ;
	}
	return sqrt(1 - prod * prod / norm_p1 / norm_p2) ;		
}

void print_usage(){
	printf(
			"pagerank module, %d-bit version\n"
			"usage: %s {n} {fn_link} {fn_escape} {fn_output} {alpha}\n" 
			"    {n}: node number. node id is in range(1,n)\n"
			"    {fn_link}: the 'link' file from raw data\n"
			"    {fn_escape}: the escape vector file\n"
			"    {fn_output}: the probability output file\n" 
			"    {alpha}: the transfer ratio (0,1). (1-alpha) will escape \n" 
			, (int)(sizeof(int*) * 8)
			, g_execute.c_str() 
		  ) ;
}

void init(){
	//reset global data
	g_adj_list.clear() ;
	g_escape_vector.clear() ;
	g_probability.clear() ;
//	g_name2index.clear() ;
//	g_index2name.clear() ;
	//g_node_count = 0 ;

	//read link file 
	char from[MAX_LINE] = {'\0'} ;
	char to[MAX_LINE] = {'\0'} ;
	char buffer[MAX_LINE] = {'\0'} ;
	double weight = 0 ;
	FILE *fp_link = fopen(g_fn_link.c_str(), "r") ;
	if ( NULL == fp_link ){
		exit(ERR_FILE_OPEN) ;
	}
	//build string to id mapping
	//while ( 2 == fscanf(fp_link, "%s%s", from, to) ){
	int fnum = 0 ;
	//while ( (fnum = fscanf(fp_link, "%s%s%lf", from, to, &weight)) >= 2 ){
	//while ( fgets(buffer, MAX_LINE, fp_link) ){
	//	fnum = sscanf(buffer , "%s%s%lf", from, to, &weight) ;
	//	if ( ! g_name2index.count(from) ){
	//		g_name2index[from] = g_node_count ;
	//		g_index2name[g_node_count] = from ;
	//		g_node_count ++ ;
	//	}					
	//	if ( ! g_name2index.count(to) ){
	//		g_name2index[to] = g_node_count ;
	//		g_index2name[g_node_count] = to ;
	//		g_node_count ++ ;
	//	}					
	//}

	//pre-allocate space
	g_adj_list.reserve(g_node_count) ;
	g_escape_vector.reserve(g_node_count) ;
	g_probability.reserve(g_node_count) ;
	for ( int i = 0 ; i < g_node_count ; i ++ ){
		g_adj_list.push_back(list<node_t>()) ;
		g_escape_vector.push_back(0.0) ;
		g_probability.push_back(0.0) ;
	}

	//build adjacent list
	rewind(fp_link) ;
	//while ( (fnum = fscanf(fp_link, "%s%s%lf", from, to, &weight)) >= 2  ){
	while ( fgets(buffer, MAX_LINE, fp_link) ){
		fnum = sscanf(buffer , "%s%s%lf", from, to, &weight) ;
		//int from_index = g_name2index[from] ;
		//int to_index = g_name2index[to] ;
		int from_index = atoi(from) - 1 ;
		int to_index = atoi(to) - 1 ;
		node_t tmp ;
		tmp.id = to_index ;
		if ( fnum == 3 ) {
			tmp.weight = weight ;
		} else {
			tmp.weight = 1 ;	
		}
		g_adj_list[from_index].push_back(tmp) ;
	}
	fclose(fp_link) ;

	//normalize adjacent list weights 
	for ( unsigned int from = 0 ; from < g_adj_list.size() ; from ++ ){
		list<node_t> &l = g_adj_list[from] ;
		double total_weight = 0.0 ;
		for ( list<node_t>::iterator it = l.begin() ;
				it != l.end() ; it ++ ){
			//node_t &tmp = *it ;
			total_weight += it->weight ;
		}
		for ( list<node_t>::iterator it = l.begin() ;
				it != l.end() ; it ++ ){
			it->weight /= total_weight;
		}
	}

	//below

	//read escape vector file
	//for ( vector<double>::iterator it = g_escape_vector.begin() ;
	//		it != g_escape_vector.end() ; it ++ ){
	//	*it = 0 ;	
	//}
	FILE *fp_escape = fopen(g_fn_escape.c_str(), "r") ;
	if ( NULL == fp_escape ){
		exit(ERR_FILE_OPEN) ;
	}
	double total_weight = 0 ;
	//double weight = 0 ;
	char node[MAX_LINE] = {'\0'} ;
	while ( fgets(buffer, MAX_LINE, fp_link) ){
		if ( 2 == sscanf(buffer, "%s%lf", node, &weight) ) {
			//while ( 2 == fscanf(fp_escape, "%s%lf", node, &weight)) {
			//int index = g_name2index[node] ;
			int index = atoi(node) - 1 ;
			g_escape_vector[index] = weight ;
			total_weight += weight ;
		} else {
			break ;
		}
	}
	fclose(fp_escape) ;
	if ( total_weight < eps ) {
		exit(ERR_ZERO_WEIGHT) ;
	}
	//above
	
	//normalize weight 
	for ( vector<double>::iterator it = g_escape_vector.begin() ;
			it != g_escape_vector.end() ; it ++ ){
		*it /= total_weight ;
	}

	//initialize probability
	//may use some heuristic here to accelerate convergence
	//the first version just make uniform distribution
	for ( vector<double>::iterator it = g_probability.begin() ;
			it != g_probability.end() ; it ++ ){
		*it = 1.0 / g_node_count ;
	}
}

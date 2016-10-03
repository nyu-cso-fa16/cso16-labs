/* Match every k-character snippet of the query_doc document
	 among a collection of documents doc1, doc2, ....

	 ./rkmatch snippet_size query_doc doc1 [doc2...]

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <assert.h>
#include <time.h>
#include <ctype.h>

#include "bloom.h"
#include "rkmatch.h"

enum algotype { EXACT=0, SIMPLE, RK, RKBATCH};

int 
main(int argc, char **argv)
{
	int k = 20; /* default match size is 20*/
	int which_algo = SIMPLE; /* default match algorithm is simple */

	
	/* Refuse to run on platform with a different size for long long*/
	assert(sizeof(long long) == 8);

	/*getopt is a C library function to parse command line options */
	int c;
	while ((c = getopt(argc, argv, "t:k:q:")) != -1) {
	       	switch (c) {
			case 't':
				/*optarg is a global variable set by getopt() 
					it now points to the text following the '-t' */
				which_algo = atoi(optarg);
				break;
			case 'k':
				k = atoi(optarg);
				break;
			case 'q':
				BIG_PRIME = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Valid options are: -t <algo type> -k <match size> -q <prime modulus>\n");
				exit(1);
	       	}
       	}

	/* optind is a global variable set by getopt() 
		 it now contains the index of the first argv-element 
		 that is not an option*/
	if (argc - optind < 1) {
		printf("Usage: ./rkmatch query_doc doc\n");
		exit(1);
	}

	unsigned char *qdoc, *doc; 
	int qdoc_len, doc_len;
	/* argv[optind] contains the query_doc argument */
	read_file(argv[optind], &qdoc, &qdoc_len); 
	qdoc_len = normalize(qdoc, qdoc_len);

	/* argv[optind+1] contains the doc argument */
	read_file(argv[optind+1], &doc, &doc_len);
	doc_len = normalize(doc, doc_len);

	int num_matched;
	switch (which_algo) {
            case EXACT:
                if (exact_match(qdoc, qdoc_len, doc, doc_len)) 
                    printf("Exact match\n");
                else
                    printf("Not an exact match\n");
                break;
	   
	    case SIMPLE:
	       	/* for each chunk of qdoc (out of qdoc_len/k chunks of qdoc, 
		 * check if it appears in doc as a substring */
		num_matched = 0;
		 for (int i = 0; (i+k) <= qdoc_len; i += k) {
			 if (simple_substr_match(qdoc+i, k, doc, doc_len)) {
				 num_matched++;
			 }
			
		 }
		 printf("%d chunks matched (out of %d), percentage: %.2f\n", \
				 num_matched, qdoc_len/k, (double)num_matched/(qdoc_len/k));
		 break;
	   
	    case RK:
		 /* for each chunk of qdoc (out of qdoc_len/k in total), 
		  * check if it appears in doc as a substring using 
		  * the rabin-karp substring matching algorithm */
		 num_matched = 0;
		 for (int i = 0; (i+k) <= qdoc_len; i += k) {
			 if (rabin_karp_match(qdoc+i, k, doc, doc_len)) {
				 num_matched++;
			 }
		 }
		 printf("%d chunks matched (out of %d), percentage: %.2f\n", \
				 num_matched, qdoc_len/k, (double)num_matched/(qdoc_len/k));
		 break;
	   
	    case RKBATCH:
		 /* match all qdoc_len/k chunks simultaneously (in batch) by using a bloom filter*/
		 num_matched = rabin_karp_batchmatch(((qdoc_len*10/k)>>3)<<3, k, \
				 qdoc, qdoc_len, doc, doc_len);
		 printf("%d chunks matched (out of %d), percentage: %.2f\n", \
				 num_matched, qdoc_len/k, (double)num_matched/(qdoc_len/k));
		 break;
	   
	    default :
		 fprintf(stderr,"Wrong algorithm type, choose from 0 1 2 3\n");
		 exit(1);
	}

	free(qdoc);
	free(doc);

	return 0;
}

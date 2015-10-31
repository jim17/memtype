#if defined(WIN32) || defined(WIN64)
#pragma warning(disable : 4996)
#include <conio.h>
#endif

#include <stdio.h>
#ifdef linux
#include <curses.h>
#endif
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
//#define SXMLC_UNICODE
#include "../sxmlc.h"
#include "../sxmlsearch.h"

void test_gen(void)
{
	XMLNode *node, *node1;
	XMLDoc doc;
	FILE* f;
	
	XMLDoc_init(&doc);

	node = XMLNode_alloc();
	XMLNode_set_tag(node, C2SX("xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\""));
	XMLNode_set_type(node, TAG_INSTR);
	XMLDoc_add_node(&doc, node);
	
	node = XMLNode_alloc();
	XMLNode_set_tag(node, C2SX(" Pre-comment "));
	XMLNode_set_type(node, TAG_COMMENT);
	XMLDoc_add_node(&doc, node);
	
	node = XMLNode_alloc();
	XMLNode_set_tag(node, C2SX("\nAnother one\nMulti-line...\n"));
	XMLNode_set_type(node, TAG_COMMENT);
	XMLDoc_add_node(&doc, node);
	
	node = XMLNode_alloc();
	XMLNode_set_tag(node, C2SX("properties"));
	XMLNode_set_type(node, TAG_FATHER);
	XMLDoc_add_node(&doc, node); // Becomes root node
	
	node = XMLNode_alloc();
	XMLNode_set_type(node, TAG_COMMENT);
	XMLNode_set_tag(node, C2SX("Hello World!"));
	XMLDoc_add_child_root(&doc, node);
	
	node = XMLNode_alloc();
	XMLNode_set_tag(node, C2SX("data"));
	XMLNode_set_attribute(node, C2SX("type"), C2SX("code"));
	XMLNode_set_text(node, C2SX("a >= b && b <= c"));
	XMLDoc_add_child_root(&doc, node);
	
	node = XMLNode_alloc();
	XMLNode_set_tag(node, C2SX("structure1"));
	XMLNode_set_attribute(node, C2SX("name"), C2SX("spatioconf"));
	XMLDoc_add_child_root(&doc, node);
	
	node1 = XMLNode_alloc();
	XMLNode_set_tag(node1, C2SX("structure2"));
	XMLNode_set_attribute(node1, C2SX("name"), C2SX("files"));
	XMLNode_add_child(node, node1);
	
	node = XMLNode_alloc();
	XMLNode_set_tag(node, C2SX("property3"));
	XMLNode_set_attribute(node, C2SX("name"), C2SX("aaa"));
	XMLNode_set_attribute(node, C2SX("value"), C2SX("<truc>"));
	XMLNode_add_child(node1, node);
	
	node = XMLNode_alloc();
	XMLNode_set_tag(node, C2SX("property4"));
	XMLNode_set_attribute(node, C2SX("name"), C2SX("bbb"));
	XMLNode_set_attribute(node, C2SX("readonly"), C2SX("true"));
	XMLNode_set_attribute(node, C2SX("value"), C2SX("txt"));
	XMLNode_remove_attribute(node, XMLNode_search_attribute(node, C2SX("readonly"), 0));
	XMLNode_add_child(node1, node);
	node->attributes[1].active = false;
	
	node = XMLNode_alloc();
	XMLNode_set_tag(node, C2SX("structure5"));
	XMLNode_set_attribute(node, C2SX("name"), C2SX("conf2"));
	XMLDoc_add_child_root(&doc, node);
	node->active = false;
	
	node1 = XMLNode_alloc();
	XMLNode_set_tag(node1, C2SX("property6"));
	XMLNode_set_attribute(node1, C2SX("name"), C2SX("ddd"));
	XMLNode_set_attribute(node1, C2SX("readonly"), C2SX("false"));
	XMLNode_set_attribute(node1, C2SX("value"), C2SX("machin2"));
	XMLNode_add_child(node, node1);
	
	node = XMLNode_alloc();
	XMLNode_set_tag(node, C2SX("property7"));
	XMLNode_set_attribute(node, C2SX("name"), C2SX("eee"));
	XMLNode_set_attribute(node, C2SX("value"), C2SX("machin3"));
	XMLDoc_add_child_root(&doc, node);

#if defined(WIN32) || defined(WIN64)
	//f = fopen("G:\\Code\\Workspace\\sxmlc\\data\\testout.xml", "w+t");
	f = fopen("D:\\Signalis\\Sources\\sxmlc\\data\\testout.xml", "w+t");
#else
	f = fopen("/home/matth/Code/workspace/sxmlc/data/testout.xml", "w+t");
#endif
	if (f == NULL) f = stdout;
	XMLDoc_print(&doc, f, C2SX("\n"), C2SX("    "), false, 0, 4);
	if (f != stdout) fclose(f);

	XMLDoc_free(&doc);
}

void test_DOM(void)
{
	FILE* f = NULL;
	XMLDoc doc;

	XMLDoc_init(&doc);

#if defined(WIN32) || defined(WIN64)
	if (!XMLDoc_parse_file_DOM(C2SX("D:\\Signalis\\Sources\\sxmlc\\data\\test.xml"), &doc))
#else
	if (!XMLDoc_parse_file_DOM(C2SX("/home/matth/Code/workspace/sxmlc/data/test.xml"), &doc))
#endif
		printf("Error while loading\n");
#if defined(WIN32) || defined(WIN64)
	f = fopen("D:\\Signalis\\Sources\\sxmlc\\data\\testout.xml", "w+t");
#else
	f = fopen("/home/matth/Code/workspace/sxmlc/data/testout.xml", "w+t");
#endif
	if (f == NULL) f = stdout;
	XMLDoc_print(&doc, f, C2SX("\n"), C2SX("\t"), false, 0, 4);
	if (f != stdout) fclose(f);
	XMLDoc_free(&doc);
}

void test_unicode(void)
{
	FILE* f = NULL;
	XMLDoc doc;
	SXML_CHAR* mode = C2SX("w+t");

	XMLDoc_init(&doc);

#if defined(WIN32) || defined(WIN64)
	if (!XMLDoc_parse_file_DOM(C2SX("D:\\Signalis\\Sources\\sxmlc\\data\\wordutf8.txt"), &doc))
	//if (!XMLDoc_parse_file_DOM(C2SX("D:\\Sources\\sxmlc\\data\\test.xml"), &doc))
#else
	if (!XMLDoc_parse_file_DOM(C2SX("/home/matth/Code/workspace/sxmlc/data/test.xml"), &doc))
#endif
		printf("Error while loading\n");
#ifdef SXMLC_UNICODE
	if (doc.bom_type != BOM_NONE && doc.bom_type != BOM_UTF_8) mode = C2SX("w+b");
#endif
#if defined(WIN32) || defined(WIN64)
	f = sx_fopen(C2SX("D:\\Signalis\\Sources\\sxmlc\\data\\testout.xml"), mode);
#else
	f = fopen("/home/matth/Code/workspace/sxmlc/data/testout.xml", "w+t");
#endif
	if (f == NULL) f = stdout;
	XMLDoc_print(&doc, f, C2SX("\n"), C2SX("\t"), false, 0, 4);
	if (f != stdout) sx_fclose(f);
	XMLDoc_free(&doc);
}


typedef struct _sxs {
	int n_nodes;
	int n_match;
	XMLSearch search;
} SXS;

int inc_node(const XMLNode* node, SAX_Data* sd)
{
	SXS* sxs = (SXS*)sd->user;

	sxs->n_nodes++;
	if (XMLSearch_node_matches(node, &sxs->search)) sxs->n_match++;

	return true;
}

void test_speed_SAX(void)
{
	SAX_Callbacks sax;
	SXS sxs;
	clock_t t0;

	SAX_Callbacks_init(&sax);
	sax.start_node = inc_node;
	sxs.n_nodes = 0;
	sxs.n_match = 0;
	XMLSearch_init(&sxs.search);
	XMLSearch_search_set_tag(&sxs.search, C2SX("incategory"));
	XMLSearch_search_add_attribute(&sxs.search, C2SX("category"), C2SX("category*"), true);
	printf("[SAX] Loading...\n");
	t0 = clock();
	if (!XMLDoc_parse_file_SAX(C2SX("/home/matth/Code/tmp/big.xml"), &sax, &sxs))
		printf("Error while loading\n");
	printf("[SAX] Loaded %d nodes in %d ms, found %d match\n", sxs.n_nodes, (int)((1000.0f * (clock() - t0)) / CLOCKS_PER_SEC), sxs.n_match);
	XMLSearch_free(&sxs.search, false);
}

void test_speed_DOM(void)
{
	XMLDoc doc;
	XMLSearch search;
	XMLNode* node;
	int n_match;
	clock_t t0, t1;

	XMLDoc_init(&doc);

	printf("[DOM] Loading...\n");
	t0 = clock();
	if (!XMLDoc_parse_file_DOM(C2SX("/home/matth/Code/tmp/big.xml"), &doc))
		printf("Error while loading\n");
	t1 = clock();
	printf("[DOM] Loaded in %d ms\n", (int)((1000.0f * (t1 - t0)) / CLOCKS_PER_SEC));
	XMLSearch_init(&search);
	XMLSearch_search_set_tag(&search, C2SX("incategory"));
	XMLSearch_search_add_attribute(&search, C2SX("category"), C2SX("category*"), true);
	n_match = 0;
	node = XMLDoc_root(&doc); //doc.nodes[doc.i_root];
	printf("[DOM] Searching...\n");
	t0 = clock();
	while ((node = XMLSearch_next(node, &search)) != NULL) {
		n_match++;
	}
	printf("[DOM] Found %d matching nodes in %d ms\n", n_match, (int)((1000.0f * (clock() - t0)) / CLOCKS_PER_SEC));
	XMLSearch_free(&search, false);
	t0 = clock();
	XMLDoc_free(&doc);
	printf("[DOM] Freed in %d ms\n", (int)((1000.0f * (clock() - t0)) / CLOCKS_PER_SEC));
}

static const char* tag_type_names[] = {
	"TAG_NONE",
	"TAG_PARTIAL",
	"TAG_FATHER",
	"TAG_SELF",
	"TAG_END",
	"TAG_PROLOG",
	"TAG_COMMENT",
	"TAG_CDATA",
	"TAG_DOCTYPE"
};

int start_node(const XMLNode* node, SAX_Data* sd)
{
	int i;
	printf("Start node %s <%s>\n", node->tag_type == TAG_USER+1 ? "MONTAG" : tag_type_names[node->tag_type], node->tag);
	for (i = 0; i < node->n_attributes; i++)
		printf("\t%s=\"%s\"\n", node->attributes[i].name, node->attributes[i].value);
	return true;
}

int end_node(const XMLNode* node, SAX_Data* sd)
{
	printf("End node %s <%s>\n", node->tag_type == TAG_USER+1 ? "MONTAG" : tag_type_names[node->tag_type], node->tag);
	return true;
}

int new_text(const SXML_CHAR* text, SAX_Data* sd)
{
	SXML_CHAR* p = (SXML_CHAR*)text;
	while(*p && sx_isspace(*p++)) ;
	if (*p)
		sx_printf(C2SX("Text: [%s]\n"), text);
	return true;
}

int allin1(XMLEvent event, const XMLNode* node, SXML_CHAR* text, const int n, SAX_Data* sd)
{
	switch(event) {
		case XML_EVENT_START_DOC: printf("Document start\n\n"); return true;
		case XML_EVENT_START_NODE: return start_node(node, sd);
		case XML_EVENT_END_NODE: return end_node(node, sd);
		case XML_EVENT_TEXT: return new_text(text, sd);
		case XML_EVENT_ERROR: printf("%s:%d: ERROR %d\n", sd->name, sd->line_num, n); return true;
		case XML_EVENT_END_DOC: printf("\nDocument end\n"); return true;
		default: return true;
	}
}

void test_SAX(void)
{
	SAX_Callbacks sax;

	SAX_Callbacks_init(&sax);
	//sax.start_node = start_node;
	//sax.end_node = end_node;
	//sax.new_text = new_text;
	sax.all_event = allin1;
#if defined(WIN32) || defined(WIN64)
	if (!XMLDoc_parse_file_SAX(C2SX("G:\\Code\\workspace\\sxmlc\\data\\test.xml"), &sax, NULL))
#else
	if (!XMLDoc_parse_file_SAX(C2SX("/home/matth/Code/workspace/sxmlc/data/test.xml"), &sax, NULL))
#endif
		printf("Error while loading\n");
}

void test_SAX_buffer(void)
{
	SAX_Callbacks sax;

	SAX_Callbacks_init(&sax);
	//sax.start_node = NULL;//start_node;
	//sax.end_node = NULL;//end_node;
	//sax.new_text = NULL;//new_text;
	sax.all_event = allin1;
	if (!XMLDoc_parse_buffer_SAX(C2SX("<xml><a>text</a><b name='matth'/></xml>"), C2SX("Buffer1"), &sax, NULL))
		printf("Error while loading\n");
}

int depth, max_depth;
int my_start(const XMLNode* node, SAX_Data* sd)
{
	if(++depth > max_depth) max_depth = depth;
	return DOMXMLDoc_node_start(node, sd);
}
int my_end(const XMLNode* node, SAX_Data* sd)
{
	depth--;
	return DOMXMLDoc_node_end(node, sd);
}
void test_DOM_from_SAX(void)
{
	DOM_through_SAX dom;
	SAX_Callbacks sax;
	XMLDoc doc;

	XMLDoc_init(&doc);
	dom.doc = &doc;
	dom.current = NULL;
	SAX_Callbacks_init(&sax);
	sax.start_node = my_start;
	sax.end_node = my_end;
	sax.new_text = DOMXMLDoc_node_text;
	depth = max_depth = 0;
	if (!XMLDoc_parse_file_SAX(C2SX("/home/matth/Code/tmp/big.xml"), &sax, &dom))
		printf("Failed\n");
	//XMLDoc_print(&doc, stdout, "\n", "  ", 0, 4);
	XMLDoc_free(&doc);
	printf("Max depth: %d\n", max_depth);
}

#define N 10000

void test_mem(void)
{
	static XMLDoc doc[N];
	int i, len;
	SXML_CHAR* p;
	//FILE* f = fopen("G:\\Code\\Workspace\\sxmlc\\data\\simple.xml", "rt");
	FILE* f = fopen("D:\\Sources\\sxmlc\\data\\test.xml", "rt");

	fseek(f, 0, SEEK_END);
	len = ftell(f);
	p = (SXML_CHAR*)malloc(len+sizeof(SXML_CHAR));
	fseek(f, 0, SEEK_SET);
	fread(p, 1, len, f);
	fclose(f);
	p[len] = 0;
	printf("Reading %d files:\n", N);
	_getch();
	for (i = 0; i < N; i++) {
		XMLDoc_init(&doc[i]);
		//XMLDoc_parse_file_DOM("G:\\Code\\Workspace\\XML Benchmark\\Data\\big.xml", &doc[i]);
		//XMLDoc_parse_file_DOM("G:\\Code\\Workspace\\sxmlc\\data\\simple.xml", &doc[i]);
		XMLDoc_parse_buffer_DOM(p, C2SX("simple"), &doc[i]);
		if (i % 10 == 0) printf(".");
	}
	free(p);
	printf("\nFreeing %d files:\n", N);
	_getch();
	for (i = 0; i < N; i++) {
		XMLDoc_free(&doc[i]);
		if (i % 10 == 0) printf(".");
	}
	printf("\nDone!\n", N);
}

int DS(SAX_Data* sd)
{
	return true;
}
int NS(const XMLNode* node, SAX_Data* sd)
{
	XMLDoc* doc = (XMLDoc*)sd->user;
	XMLDoc_add_node(doc, XMLNode_dup(node, false));
	if (doc->n_nodes >= 10000) {
		XMLDoc_free(doc);
		XMLDoc_init(doc);
	}
	return true;
}
int NE(const XMLNode* node, SAX_Data* sd)
{
	return true;
}
int NT(SXML_CHAR* text, SAX_Data* sd)
{
	return true;
}
int DE(SAX_Data* sd)
{
	return true;
}
int ER(ParseError err_num, int line_num, SAX_Data* sd)
{
	return true;
}

void test_mem2(void)
{
	SAX_Callbacks sax;
	int i;
	XMLDoc doc;
	SXML_CHAR* p = sx_strdup(C2SX("<tag3456789 att3456789='val3456789'>0123456789</tag3456789>"));

	SAX_Callbacks_init(&sax);
	XMLDoc_init(&doc);
	sax.start_doc = DS;
	sax.start_node = NS;
	sax.end_node = NE;
	sax.new_text = NT;
	sax.end_doc = DE;
	sax.on_error = ER;

	printf("Reading %d files:\n", N);
	_getch();
	for (i = 0; i < N; i++) {
		XMLDoc_parse_buffer_SAX(p, C2SX("simple"), &sax, &doc);
		if (i % 1000 == 0) printf(".");
	}
	free(p);
	printf("\nFreeing...\n", N);
	_getch();
	XMLDoc_free(&doc);
	printf("\nDone!\n", N);
}

void test_mem3(void)
{
	static XMLNode nodes[N];
	SXML_CHAR* p = sx_strdup(C2SX("<tag3456789 att3456789='val3456789'>0123456789</tag3456789>"));
	int i;

	printf("Reading %d nodes:\n", N);
	_getch();
	for (i = 0; i < N; i++) {
		XMLNode_init(&nodes[i]);
		XML_parse_1string(p, &nodes[i]);
		if (i % 1000 == 0) printf(".");
	}
	free(p);
	printf("\nFreeing...\n", N);
	_getch();
	for (i = 0; i < N; i++) {
		XMLNode_free(&nodes[i]);
		if (i % 1000 == 0) printf(".");
	}
	printf("\nDone!\n", N);
}

void test_search(void)
{
	XMLDoc doc;
	XMLSearch search[3];
	XMLNode* node;
	SXML_CHAR* xpath = NULL;

	XMLDoc_init(&doc);

	XMLSearch_init(&search[0]);
	XMLSearch_init(&search[1]);
	XMLSearch_init(&search[2]);

	XMLSearch_search_set_tag(&search[0], C2SX("st*"));
	XMLSearch_search_add_attribute(&search[0], C2SX("name"), C2SX("st*sub*"), true);
	XMLSearch_search_add_attribute(&search[0], C2SX("valid"), C2SX("false"), false);
	//XMLSearch_search_set_text(&search[0], "*inside *");

	XMLSearch_search_set_tag(&search[1], C2SX("property"));
	XMLSearch_search_add_attribute(&search[1], C2SX("name"), C2SX("t?t?"), true);

	XMLSearch_search_set_children_search(&search[0], &search[1]);

#if defined(WIN32) || defined(WIN64)
	if (!XMLDoc_parse_file_DOM(C2SX("G:\\Code\\workspace\\sxmlc\\data\\test.xml"), &doc)) {
#else
	if (!XMLDoc_parse_file_DOM(C2SX("/home/matth/Code/workspace/sxmlc/data/test.xml"), &doc)) {
#endif
		printf("Error while loading\n");
		return;
	}

	sx_printf(C2SX("Start search '%s'\n"), XMLSearch_get_XPath_string(&search[0], &xpath, C2SX('\"')));
	free(xpath);
	node = XMLDoc_root(&doc); //doc.nodes[doc.i_root];
	while ((node = XMLSearch_next(node, &search[0])) != NULL) {
		printf("Found match: ");
		XMLNode_print(node, stdout, NULL, NULL, false, 0, 0);
		printf("\n");
	}
	printf("End search\n");

	XMLSearch_free(&search[0], false);

	XMLDoc_free(&doc);
}

void test_xpath(void)
{
	XMLSearch search;
	SXML_CHAR* xpath2 = NULL;
	const SXML_CHAR xpath[] = C2SX("/tagFather[@name, @id!='0', .='toto*']/tagChild[.='text', @attrib='value']");

	if (XMLSearch_init_from_XPath(xpath, &search))
		printf("%s\n %s\n", xpath, XMLSearch_get_XPath_string(&search, &xpath2, '\''));
	else
		printf("Error\n");
	XMLSearch_free(&search, true);
}

static void tstre(SXML_CHAR* s, SXML_CHAR* p)
{
	if (regstrcmp(s, p))
		printf("'%s' and '%s' match\n", s, p);
	else
		printf("'%s' and '%s' DON'T match\n", s, p);
}

void test_regexp(void)
{
	tstre(C2SX("abc123"), C2SX("*"));
	tstre(C2SX("abc123"), C2SX("abc123"));
	tstre(C2SX("abc123"), C2SX("abc123*"));
	tstre(C2SX("abc123"), C2SX("aXc123"));
	tstre(C2SX("abc123"), C2SX("a?c123"));
	tstre(C2SX("abc123"), C2SX("abc*"));
	tstre(C2SX("abc123"), C2SX("*123"));
	tstre(C2SX("abc123"), C2SX("a*3"));
	tstre(C2SX("abc123"), C2SX("a*1?3"));
	tstre(C2SX("abc1X3"), C2SX("a*1?3"));
	tstre(C2SX("abc123"), C2SX("a*1?4?"));
	tstre(C2SX("abc123"), C2SX("a*1?3*"));
	tstre(C2SX("ab?123"), C2SX("a*1?3*"));
	tstre(C2SX("ab\\123"), C2SX("ab\\\\123"));
	tstre(C2SX("ab?123"), C2SX("ab\\?12*"));
	tstre(C2SX("st2sub1"), C2SX("st?sub*"));
}

void print_split(SXML_CHAR* str)
{
	int i, l0, l1, r0, r1;

	if (split_left_right(str, C2SX('='), &l0, &l1, &i, &r0, &r1, true, true)) {
		printf("[%s]%s - Left[%d;%d]: [", str, (i < 0 ? " (no sep)": ""), l0, l1);
		for (i = l0; i <= l1; i++) sx_fputc(str[i], stdout);
		printf("], right[%d;%d]: [", r0, r1);
		for (i = r0; i <= r1; i++) sx_fputc(str[i], stdout);
		printf("]\n");
	}
	else
		printf("Malformed [%s]\n", str);
}
void test_split(void)
{
	print_split(C2SX("attrib=\"value\""));
	print_split(C2SX("attrib = \"value\""));
	print_split(C2SX("'attrib' = 'va\\'lue'"));
	print_split(C2SX("'attri\\'b ' = 'va\\'lue'"));
	print_split(C2SX(" attrib = 'va'lue'"));
	print_split(C2SX("\"att\"rib \" = \"val\\\"ue\""));
	print_split(C2SX("attrib = \"value\""));
	print_split(C2SX("attrib=\" val ue \""));
	print_split(C2SX("attrib="));
	print_split(C2SX("attrib=''"));
	print_split(C2SX("attrib"));
}

void test_NodeXPath(void)
{
	XMLNode node, node1;
	SXML_CHAR* buf;

	XMLNode_init(&node);
	XMLNode_set_tag(&node, C2SX("monroot"));

	XMLNode_init(&node1);
	XMLNode_set_tag(&node1, C2SX("montag"));
	XMLNode_set_text(&node1, C2SX("This <is> \"some\" text & chars"));
	XMLNode_set_attribute(&node1, C2SX("name"), C2SX("first one"));
	XMLNode_set_attribute(&node1, C2SX("readonly"), C2SX("fa<l>se"));
	XMLNode_set_attribute(&node1, C2SX("value"), C2SX("T\"B\"D"));

	XMLNode_add_child(&node, &node1);

	buf = NULL;
	sx_printf(XMLNode_get_XPath(&node1, &buf, true));
	free(buf);
}

void test_search_attribute(void)
{
	XMLNode node;
	SXML_CHAR* val;

	XMLNode_init(&node);
	XMLNode_set_attribute(&node, C2SX("name"), C2SX("first one"));
	XMLNode_set_attribute(&node, C2SX("readonly"), C2SX("false"));
	XMLNode_set_attribute(&node, C2SX("value"), C2SX("TBD"));

	XMLNode_get_attribute(&node, C2SX("value"), &val);
	sx_printf(C2SX("'value' without default: [%s]\n"), val);
	__free(val);

	XMLNode_get_attribute_with_default(&node, C2SX("value2"), &val, C2SX("defval"));
	sx_printf(C2SX("'value2' with default 'defval' [%s]\n"), val);
	__free(val);
}

#if 1
int main(int argc, char** argv)
{
	XML_register_user_tag(TAG_USER+1, C2SX("<#[MONTAG-"), C2SX("-]>"));
	//test_gen();
	//test_unicode();
	//test_DOM();
	//test_SAX();
	//test_SAX_buffer();
	//test_DOM_from_SAX();
	//test_search();
	test_xpath();
	//test_regexp();
	//test_split();
	//test_speed_DOM();
	//test_speed_SAX();
	//test_NodeXPath();
	//test_mem();
	test_search_attribute();

#if defined(WIN32) || defined(WIN64)
	_getch();
#elif defined(linux)
	//getch();
#endif
	return 0;
}
#endif

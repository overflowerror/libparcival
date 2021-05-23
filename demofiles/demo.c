#include <stdio.h>

#include <templates.h>

int main() {
	char *names[] = {
		"foo",
		"bar",
		"foobar"
	};	

	renderTemplate("demo.html.templ", stdout, "User-List", names, 3);
	
	return 0;
}

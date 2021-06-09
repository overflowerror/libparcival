# libparcival

Parcival is a templating engine for C.

The template files are compiled into C files that can then be compiled with the rest of the application. To access the template code the `template.h` file is used (see "Using Templates").

## Building

`make all` will build the executeable for translating the templates and the static library that includes the template registry.

## Dependencies & Compatibility

Besides a C compiler `lex` and `yacc` are also required. The `Makefile` uses `flex` and `bison` respecively but I think I only used non-specific features.

As for the C compiler support for GNU function attributes is needed. So `gcc` or `clang` should work, maybe others as well.

# Usage

## Template Format

Template files have the following syntax:

```
[meta section]
%%
[main section]
```

The meta section can contain the parameter list for the template as well as static statement blocks for optional includes, definitions, ...

The main section contains the content of the template. Here statement blocks and output blocks are allowed.

### Parameter Block

The parameter block has the following syntax:

```
{$ [parameter list ] $}
```

The parameter list is comma-seperated list of the parameters of the template including their type. 

Example:
```
{$ char* title, user_t* users, size_t numberOfUsers $}
```

Note: All types that C does not provide by default have to be declared in a statement block in the meta section or in a header file included by a statement block in the meta section. In the example above this would apply to the type `user_t`.

Note: In case of pointer parameters they are not allowed to be modified in statement or output blocks. If the template has side effects like that, the behavior might be undefined (because of the calculation of the the output size).

### Statement Block

Statement blocks have the following syntax:

```
{% [C statements] %}
```

In the meta section these blocks can be used to define datatypes - even functions - and include any needed header files. Anything that got declared this way can be used as a parameter type in a parameter block.

Examples:
```
{% #include "someheader.h" %}
{%
struct s {
	int i;
};
%}
```

In the main section these blocks will open a template block. This template blocks has to be closed with `{% end %}`. Statement blocks in the main section are meant to be used for loops and conditions.

Example:
```
{% for(int i = 0; i < 10; i++) %}
This string will be printed 10 times.
{% end %}
```

Inside statement blocks in the main section the parameters as well as all local variables of parent statement blocks can be accessed.

### Output Block

Output blocks have the following syntax:

```
{{ [format string] {, format parameters} }}
```

These blocks are effectively `printf()`-calls. In the rendered templated they will be replaced by the corresponding output. 

Example:
```
{{ "foo: %s - %d", "bar", 69 }}
```

### Structure Block

Structure blocks have the following syntax:

```
{# [structure command] ( [parameters] ) #}
```

The following structure commands are implemented:

## Render

Using the render command, a another template can be included at the current position. The first parameter is the name of the template to be included as a string. All following parameters will be used as template parameters.

Note: The `render` command can only be used in the main section.

Example:
```
{# render("templates/index.html.templ", "Page Title", userArray, userArrayLength) #}
```

All used variables have to be declared (for example as template parameters or local variables of statement blocks).

## Extends

`extends` can be used to extend an existing abstract template (see "Child"). This is useful for splitting the page layout and the content into multiple files.

Similar to `render` command the arguments are the name of the parent template followed by the template parameters of the parent (only parameters of the current template are allowed as variables).

Note: The `extends` command can only be used in the meta section.

```
{# extends("templates/layout.html.templ", "Page Title") #}
```

## Child

`child` is used in the main section of a template to indicate where the child template should be rendered. Using this command will implicitly set the template to abstract.

```
{# child() #}
```

### Basic Example

```
{% #include "entities.h" %}
{$ char* title, struct user* users, size_t userno $}
%%
<!DOCTYPE html>
	<head>
		<title>{{ "%s", title }}</title>
	</head>
	<body>
		<ul>
			{% for (size_t i = 0; i < userno; i++) %}
				{% if (!users[i].hidden) %}
					<li>{{ "%s", users[i].name }}</li>
				{% end %}
			{% end %}
		</ul>
	</body>
</html>
```

For an example using structure blocks, please take a look at the provided demo files.

## Translating Template

To translate a template call the `parvical` binary using the template file as it's argument. The generated C code will be outputed to `stdout`.

The filename (and path) of the template file will set as the templates name. I might add a flag for setting custom template names later on.

Example:
```
./parvical templates/index.html.templ > gen/templates/index.tab.c
```

This will translate the template in `templates/index.html.templ` using the name "templates/index.html.templ" and write the generated code to `gen/templates/index.tab.c`.

## Compiling & Linking

Just include all generated files when compiling and linking your application.

The file `templates.h` needs to be accessable in the include-path. So make sure to add the `-Isrc/` flag (change the path to the correct location).

## Using Templates

Include the `templates.h` file into your program.
```
#include <templates.h>
```

The function `renderTemplate()` will render the template with the given name to a `FILE*`. The first argument is the name of the template, the second one is the `FILE*` into which the template will be rendered. The following arguments are the parameters of the template itself (in the order that they were declared in the template file).

The function `sizeTemplate()` will return the exact length of the output of a render. The first argument is the name of the template. The following arguments are the template parameters.

The function `renderTemplateStr()` will return a string containing the rendered template. The first argument is the name of the template. Followed by the template parameters. Note that the string is allocated on the heap - the result has to be freed manually.

Following example shows how this could look like (using the previous discussed "Complete Example" withe name specified in "Translating Template").
```
renderTemplate("templates/index.html.templ", stdout, "Page Title", userArray, userArrayLength);


size_t length = sizeTemplate("templates/index.html.templ", "Page Title", userArray, userArrayLength);


char* string = renderTemplateStr("templates/index.html.templ", "Page Title", userArray, userArrayLength);
free(string); // don't forget to free
```

# Demo

`make demo`

The code for this demo can be found in `demofiles/`.

# Contributing

Issue Reports and Pull Requests are welcome.

// this is {{.name}} xml

{{range .memlist}}
{{if : if data["name"] == "Enum" then return true else return false end}}
{{with .param}}#define {{.name}} ({{.type}}){{end}}
{{else}}
struct {{with .param}}{{.name}}{{end}}
{
{{range .memlist}}
	int m_{{.name}};
{{end}}
};
{{end}}
{{end}}

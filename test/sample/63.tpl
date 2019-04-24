package com.netease.game.{{.name}};

// this is {{.name}} java
public class {{.name}}Info
{
{{range .memlist}}{{if : if data["type"] == "REAL" then return true else return false end}}
	private float {{.name}};
{{end}}{{if : if data["type"] == "INT" then return true else return false end}}
	private int {{.name}};
{{end}}{{if : if data["type"] == "INTS" then return true else return false end}}
	private int[] {{.name}};
{{end}}{{end}}
}


def test_case
{"RawParseTree"=>[:cdecl, :X, [:lit, 42]],
 "Ruby"=>"X = 42",
 "RubyParser"=>s(:cdecl, :X, s(:lit, 42))}
end

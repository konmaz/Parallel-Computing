Output = readlines("./peregrine.out", keep=true)
len = length(Output)
println("Read $len lines from peregrine.out")
Passwords = []

PasswordRegex = r"[a-zA-Z0-9]*:(.*?)*(\r\n|\r|\n)"

for (i, v) in enumerate(Output)
  if match(PasswordRegex, v) != Nothing
    push!(Passwords, v)
  end
end

pwUnique = length(unique(Passwords))
println("Unique passwords: $pwUnique")

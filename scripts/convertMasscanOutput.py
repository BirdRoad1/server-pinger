import re
exp = re.compile(r"\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}")
print('Started')
with open('output.txt', 'w') as outfile:
    with open("output.xml") as infile:
        for line in infile:
            x = exp.search(line) 
            if x:
                outfile.write(x.group() + '\n')

print('All done!')
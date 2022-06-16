import re

filenames = [
'Adventures of Huckleberry Finn.txt',
'Frankenstein.txt',
'Pride and Prejudice.txt',
'The Kama Sutra of Vatsyayana.txt',
'Alices Adventures in Wonderland.txt',
'Grimms Fairy Tales.txt',
'The Adventures of Sherlock Holmes.txt',
'The Prince.txt',
'Gullivers Travels.txt',
'The Divine Comedy.txt'
]

wordsList = set()

for fileName in filenames:
    with open(fileName) as f:
        res = set(re.findall("[a-zA-Z\-\.'/]+", f.read()))
        res2 = [re.sub(r'[^a-zA-Z]','',x.lower()) for x in res]
        wordsList.update(res2)
        #f1.write('\n'.join(res2))


with open('word_list.txt', 'w') as f1:
    for _ in wordsList:
        f1.write(_+'\n')


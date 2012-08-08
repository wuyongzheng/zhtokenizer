#ifndef CONFIG_H
#define CONFIG_H

#define MINWORDLEN 2
#define MAXWORDLEN 4
#define MAXSENTENCE 1000
#define ZHLOW 0x4E00
#define ZHHIGH 0x9FFF
#define WORDSCORE(len,weight) ((len)*1100-100+(weight))

#endif

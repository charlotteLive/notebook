import requests
from bs4 import BeautifulSoup


#显示图片
%pylab inline
def show_img(img_url, name):
    data = requests.get(img_url)
    img_name = name+'.jpg'
    with open(img_name, 'wb') as f:
        f.write(data.content)
    img = imread(img_name)
    imshow(img)

#获取网页
url = 'https://movie.douban.com/top250'
data = requests.get(url)

#获取电影列表
soup = BeautifulSoup(data.text, 'lxml')
movie_list = soup.find(class_="grid_view")
movies = movie_list.find_all('li')

#获取电影信息
movie_url = []
movie_titles = []
movie_rating_num = []
movie_img = []
movie_mark = []

for m in movies:
    movie_url.append(m.find('a').get('href'))
    movie_titles.append(m.find(class_='title').get_text())
    movie_rating_num.append(m.find(class_='rating_num').get_text())
    movie_img.append(m.find('img').get('src'))
    movie_mark.append(m.find(class_='inq').get_text())


for i in range(len(movies)):
    print('标题: ' + movie_titles[i])
    print('链接: ' + movie_url[i])
    print('评分: ' + movie_rating_num[i])
    print('评论: ' + movie_mark[i])
    print('图片: ' + movie_img[i])
    #show_img(movie_img[i], movie_titles[i])
    print('\n')
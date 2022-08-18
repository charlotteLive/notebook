PyTorch深度学习









加载一批64×224×224×3的猫咪图片，其中64表示批尺寸或图片数量，两个224分别表示高和宽，3表示通道数：

```python
#从磁盘读取猫咪图片
cats = glob(data_path+'*.jpg')
#将图片转换成numpy数组
cat_imgs = np.array([np.array(Image.open(cat).resize((224,224))) for cat in
cats[:64]])
cat_imgs = cat_imgs.reshape(-1,224,224,3)
cat_tensors = torch.from_numpy(cat_imgs)
cat_tensors.size()
```
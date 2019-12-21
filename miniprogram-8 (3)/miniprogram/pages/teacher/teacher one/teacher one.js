Page({

  data: {
    tempFilePaths: null,
    accesstoken: '',
    person_num: '0'

  },
  onReady: function (e) {
    wx.request({
      url: 'https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id=iddd4hT8Kq1mszoDcL3hffLW&client_secret=QvLamoiiv9QIrzpECOxHgnIsaD5hG5Vx',
      method: 'POST',
      success: (res) => {
        console.log(res);
        var accesstoken = res.data.access_token;
        console.log(accesstoken);
        this.setData({
          accesstoken: res.data.access_token
        })
      },
    })
  },

  getResult: function (e) {
    var access_token = this.data.accesstoken;
    request_url = "https://aip.baidubce.com/rest/2.0/image-classify/v1/body_num";
    var request_url = request_url + "?access_token=" + access_token;
    var base64 = wx.getFileSystemManager().readFileSync(this.data.tempFilePaths[0], "base64");
    wx.request({
      url: request_url,
      method: 'POST',

      header: {
        'content-type': 'application/x-www-form-urlencoded'
      },
      data: { image: base64, },
      success: (res) => {
        var person_num =
          res.data.person_num;
        console.log(person_num);
        this.setData({
          person_num: res.data.person_num
        })

        console.log(res);
      },
      fail: function (err) {
        console.log(err);
      },

    })

  },



  //确定图片来源，从相册中选择或者是拍照
  chooseImage: function () {
    wx.showActionSheet({
      itemList: ['从相册中选择'],
      itemColor: "#CED63A",
      success: (res) => {
        if (res.cancel) {
          return;
        }
        if (res.tapIndex == 0) {
          this.chooseWxImage('album')
        }
      }
    })
  },

  //选择图片
  chooseWxImage: function (type) {
    wx.chooseImage({
      sizeType: ['original', 'compressed'],
      sourceType: [type],
      success: (res) => {
        this.setData({
          tempFilePaths: res.tempFilePaths,
        })
      }
    })
  },


  //上传图片至服务器并接受返回的结果
  identifyImage: function () {
    if (!this.data.tempFilePaths) {
      console.error("no selected image")
      return;
    }
    /**
     * 调用微信上传文件接口, 此处向我们的本地服务器发送请求, 故运行此代码时要确保本地服务已经启动
     */

  }
})
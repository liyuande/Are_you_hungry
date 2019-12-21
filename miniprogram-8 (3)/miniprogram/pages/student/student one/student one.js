const devicesId = "577406884" // 填写在OneNet上获得的devicesId
const api_key = "Xsu4BlA79RsYIhc=BsLaZWhUdx8=" // 填写在OneNet上的 api-key
const db = wx.cloud.database()

Page({

  data: {
    index: '',
    accesstoken: '',
    person_num: '0',
    captchaImage:'',
    trueImage:''
  },




  //onenet上获取数据流编号
  onLoad: function () 
  {
    console.log(`your deviceId: ${devicesId}, apiKey: ${api_key}`)

    //每隔60s自动获取一次数据进行更新
    const timer = setInterval(() => {
      
      this.getDatapoints().then(datapoints => {
        //this.update(datapoints)
      })

    }, 5000)

    wx.showLoading({
      title: '加载中'
    })

    this.getDatapoints().then((datapoints) => {
     
      wx.hideLoading()
      //this.firstDraw(datapoints)

    }).catch((err) => {
      
      wx.hideLoading()
      console.error(err)
      clearInterval(timer) //首次渲染发生错误时禁止自动刷新
    })


    
  },

  
  total:function(){
this.getPicture();
this.getResult();
    var person_num = this.data.person_num;
    if (!wx.cloud) {
      console.error('请使用 2.2.3 或以上的基础库以使用云能力')
    } else {
      wx.cloud.init({
        traceUser: true,
      })
    }


    //初始化数据库
    const db = wx.cloud.database()

    //向数据库添加一条记录
    db.collection('person_num').add({
      // data 字段表示需新增的 JSON 数据

      data: {
        content: person_num,
        tag: 1
      },
      success: function (res) {
        // res 是一个对象，其中有 _id 字段标记刚创建的记录的 id
        console.log(res)
      }
    })

  },
  
  getDatapoints: function () {

    return new Promise((resolve, reject) => {

      wx.request({

        url: `https://api.heclouds.com/devices/${devicesId}/datapoints?datastream_id=ov2640_jpg`,



        // 此处header与前面实验一致, 发送的请求体格式为json, 故将content-type设置为application/json



        header: {

          'content-type': 'application/json',

          'api-key': api_key

        },

        success: (res) => {

          const status = res.statusCode

          const response = res.data

          if (status !== 200) {

            reject(res.data)

          }

          if (response.errno !== 0) {

            reject(response.error)

          }

          resolve({

            ov2640_jpg: response.data.datastreams[0].datapoints.reverse(),

          })

          console.log(response.data.datastreams[0]);

          this.setData({
              index: response.data.datastreams[0].datapoints[0].value.index
            })

        },

        fail: (err) => {

          reject(err)

        }

      })
 
    })

  },

  
  getPicture:function(){

    var index = this.data.index;

    var that = this;
     
     wx.request({
      
       url:'http://api.heclouds.com/bindata/'+index,
       
       method:'GET',

       responseType:'arraybuffer',
       
       header: {

         'api-key': api_key

       },

       success: function (res) {
         console.log(res.data)
         var data = wx.arrayBufferToBase64(res.data)
         //console.log(res.statusCode)
         if (res.statusCode == 200) {
           that.setData({
             trueImage:data,
             captchaImage: 'data:image/jpg;base64,' + data,  // data 为接口返回的base64字符串
           })
         }

       }

      })

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
    var base64 = this.data.trueImage
    wx.request({
      url: request_url,
      method: 'POST',

      header: {
        'content-type': 'application/x-www-form-urlencoded'
      },
      data: { 
        image: base64
         },
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
  

  onGetOpenid: function () {
    // 调用云函数
    wx.cloud.callFunction({
      name: 'login',
      data: {},
      success: res => {
        console.log('[云函数] [login] user openid: ', res.result.openid)
        app.globalData.openid = res.result.openid
        wx.navigateTo({
          url: '../userConsole/userConsole',
        })
      },
      fail: err => {
        console.error('[云函数] [login] 调用失败', err)
        wx.navigateTo({
          url: '../deployFunctions/deployFunctions',
        })
      }
    })
  },




  
})
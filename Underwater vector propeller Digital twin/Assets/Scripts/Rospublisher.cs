using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Unity.Robotics.ROSTCPConnector;
using RosMessageTypes.UnityPropeller;
using System;
using UnityEngine.UI;

public class Rospublisher : MonoBehaviour
{
    ROSConnection ros2;
    public string topicName = "Target_pose";
    public GameObject chuanlianliangan1;
    public GameObject qudonggan1;
    public GameObject qudonggan2;
    public GameObject qudonggan3;
    public GameObject magnetic_coupling;
    public Text jizuo_real_text;
    public Text gan1_real_text;
    public Text gan2_real_text;
    public Text gan3_real_text;
    public float publishMessageFrequency = 0.001f;
    private float timeElapsed;
    // Start is called before the first frame update
    void Start()
    {
        // 启动ROS2连接
        ros2 = ROSConnection.GetOrCreateInstance();
        ros2.RegisterPublisher<TargetPoseMsg>(topicName);
        ros2.Subscribe<BackStatusMsg>("Back_status", BackStatusCallback);
    }

    // 订阅回调函数
    void BackStatusCallback(BackStatusMsg Message)
    {
        string formatted = Message.realpose_dm3410.ToString("F1");
        gan1_real_text.text = "实物反馈值：" + formatted + "度";
        string formatted_1 = Message.realpose_dm3910_1.ToString("F1");
        gan1_real_text.text = "实物反馈值：" + formatted_1 + "度";
        string formatted_2 = Message.realpose_dm3910_2.ToString("F1");
        gan1_real_text.text = "实物反馈值：" + formatted_2 + "度";
        string formatted_3 = Message.realpose_dm3910_3.ToString("F1");
        gan1_real_text.text = "实物反馈值：" + formatted_3 + "度";
    }

    // Update is called once per frame
    void Update()
    {
        timeElapsed += Time.deltaTime;
        
        if (timeElapsed > publishMessageFrequency){
            // 填充节点消息内容
            TargetPoseMsg targetPose = new TargetPoseMsg(
                (float)Math.Round(chuanlianliangan1.GetComponent<joint1_rotation>().dm3410_pose,1),
                -(float)Math.Round(qudonggan1.transform.eulerAngles.z-90.0f, 1),
                -(float)Math.Round(qudonggan2.transform.eulerAngles.z-330.0f, 1),
                -(float)Math.Round(qudonggan3.transform.eulerAngles.z-210.0f,1),
                // chuanlianliangan1.GetComponent<joint1_rotation>().dm3410_pose,
                // qudonggan1.transform.eulerAngles.z-90.0f,
                // qudonggan2.transform.eulerAngles.z-330.0f,
                // qudonggan3.transform.eulerAngles.z-210.0f,
                magnetic_coupling.GetComponent<Magnetic_coupling>().current_pwm
            );
        ros2.Publish(topicName, targetPose);       // 最终发送消息到ros中运行的 server_endpoint.py 脚本
        timeElapsed = 0;                           // 重置时间
        }
    }
}

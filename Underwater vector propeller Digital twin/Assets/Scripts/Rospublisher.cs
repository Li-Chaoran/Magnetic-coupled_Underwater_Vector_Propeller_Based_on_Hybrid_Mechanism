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
        // ����ROS2����
        ros2 = ROSConnection.GetOrCreateInstance();
        ros2.RegisterPublisher<TargetPoseMsg>(topicName);
        ros2.Subscribe<BackStatusMsg>("Back_status", BackStatusCallback);
    }

    // ���Ļص�����
    void BackStatusCallback(BackStatusMsg Message)
    {
        string formatted = Message.realpose_dm3410.ToString("F1");
        gan1_real_text.text = "ʵ�ﷴ��ֵ��" + formatted + "��";
        string formatted_1 = Message.realpose_dm3910_1.ToString("F1");
        gan1_real_text.text = "ʵ�ﷴ��ֵ��" + formatted_1 + "��";
        string formatted_2 = Message.realpose_dm3910_2.ToString("F1");
        gan1_real_text.text = "ʵ�ﷴ��ֵ��" + formatted_2 + "��";
        string formatted_3 = Message.realpose_dm3910_3.ToString("F1");
        gan1_real_text.text = "ʵ�ﷴ��ֵ��" + formatted_3 + "��";
    }

    // Update is called once per frame
    void Update()
    {
        timeElapsed += Time.deltaTime;
        
        if (timeElapsed > publishMessageFrequency){
            // ���ڵ���Ϣ����
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
        ros2.Publish(topicName, targetPose);       // ���շ�����Ϣ��ros�����е� server_endpoint.py �ű�
        timeElapsed = 0;                           // ����ʱ��
        }
    }
}

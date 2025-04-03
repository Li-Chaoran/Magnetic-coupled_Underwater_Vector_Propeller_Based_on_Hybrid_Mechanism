using System;
using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.UI;

public class Unity_On_Click : MonoBehaviour
{
    public GameObject moving_plate_add_axis;
    public GameObject moving_plate;
    public GameObject chuanlianliangan1;
    private int flag = 0;     // ����Ϊ0�������ڸ�λ
    public Text tiptext;      // ��ʾ��

    public void Backtozero()
    {
        flag = 1;            // ������λ
        tiptext.text = "���ڸ�λ�У��������";
    } 
    // Start is called before the first frame update
    void Start()
    {
        tiptext.text = "�����˴��ڿɲٿ�״̬";
    }

    // Update is called once per frame
    void Update()
    {
        // Debug.Log(chuanlianliangan1.GetComponent<joint1_rotation>().dm3410_pose);
        // ��λ����
        if (flag == 1){
            if(chuanlianliangan1.GetComponent<joint1_rotation>().dm3410_pose <= 0.0f){      // ��0-180��֮�䣬ʹ�����溯����λ
                if (Math.Abs(chuanlianliangan1.GetComponent<joint1_rotation>().dm3410_pose)>0.03f){
                    chuanlianliangan1.GetComponent<joint1_rotation>().dm3410_pose = chuanlianliangan1.GetComponent<joint1_rotation>().dm3410_pose + 0.01f;
                }
                else flag = 2;
            }
            else if(chuanlianliangan1.GetComponent<joint1_rotation>().dm3410_pose > 0.0f){
                if (Math.Abs(chuanlianliangan1.GetComponent<joint1_rotation>().dm3410_pose)>0.03f){  
                    chuanlianliangan1.GetComponent<joint1_rotation>().dm3410_pose = chuanlianliangan1.GetComponent<joint1_rotation>().dm3410_pose - 0.01f;
                }
                else flag = 2;
            }
        }
        // ��λ����ĩ��ִ����
        if (flag == 2){
            if(moving_plate_add_axis.transform.eulerAngles.x <= 180.0f){      // ��0-180��֮�䣬ʹ�����溯����λ
                if (Math.Abs(moving_plate_add_axis.transform.eulerAngles.x)>0.05f){
                    moving_plate_add_axis.transform.Rotate(new Vector3(-1.0f, 0.0f, 0.0f), (-moving_plate_add_axis.transform.eulerAngles.x)*(1*Time.deltaTime), Space.World);
                }
                else flag = 3;
                moving_plate_add_axis.GetComponent<All_end_effector>().joint1 = 180.0f+moving_plate_add_axis.transform.eulerAngles.x;
            }
            else if(180.0f<moving_plate_add_axis.transform.eulerAngles.x && moving_plate_add_axis.transform.eulerAngles.x <= 360.0f){
                if (Math.Abs(moving_plate_add_axis.transform.eulerAngles.x-360.0f)>0.05f){  // ��Ȧ��λ
                    moving_plate_add_axis.transform.Rotate(new Vector3(-1.0f, 0.0f, 0.0f), (-moving_plate_add_axis.transform.eulerAngles.x+360.0f)*(1*Time.deltaTime), Space.World);
                }
                else flag = 3;
                moving_plate_add_axis.GetComponent<All_end_effector>().joint1 = 180.0f-moving_plate_add_axis.transform.eulerAngles.x+360.0f;
            }
        }
        // ��λ��ƽ̨Z��ת��
        if (flag == 3){
            if(moving_plate.transform.eulerAngles.z <= 90.0f){      // ��0-180��֮�䣬ʹ�����溯����λ
                if (Math.Abs(moving_plate.transform.eulerAngles.z-90.0f)>0.05f){
                    moving_plate.transform.Rotate(new Vector3(0.0f, 0.0f, -1.0f), (moving_plate.transform.eulerAngles.z-90.0f)*(0.5f*Time.deltaTime), Space.World);
                }
                else {flag = 4;moving_plate.GetComponent<Moving_plate>().z_rotate_sum = 0.0f;}
            }
            else if(moving_plate.transform.eulerAngles.z > 90.0f){
                if (Math.Abs(moving_plate.transform.eulerAngles.z-90.0f)>0.05f){  // ��Ȧ��λ
                    moving_plate.transform.Rotate(new Vector3(0.0f, 0.0f, 1.0f), (-moving_plate.transform.eulerAngles.z+90.0f)*(0.5f*Time.deltaTime), Space.World);
                }
                else {flag = 4;moving_plate.GetComponent<Moving_plate>().z_rotate_sum = 0.0f;}
            }
        }
        // ��λ��ƽ̨Y��ת��
        if (flag == 4){
            if(moving_plate.transform.eulerAngles.y <= 180.0f){      // ��0-180��֮�䣬ʹ�����溯����λ
                if (Math.Abs(moving_plate.transform.eulerAngles.y)>0.05f){
                    moving_plate.transform.Rotate(new Vector3(0.0f, -1.0f, 0.0f), moving_plate.transform.eulerAngles.y*(0.5f*Time.deltaTime), Space.World);
                }
                else {
                    flag = 0;
                    tiptext.text = "�����˴��ڿɲٿ�״̬";
                    moving_plate.GetComponent<Moving_plate>().y_rotate_sum = 0.0f;
                }
            }
            else if(180.0f<moving_plate.transform.eulerAngles.y && moving_plate.transform.eulerAngles.y <= 360.0f){
                if (Math.Abs(moving_plate.transform.eulerAngles.y-360.0f)>0.05f){  // ��Ȧ��λ
                    moving_plate.transform.Rotate(new Vector3(0.0f, 1.0f, 0.0f), (-moving_plate.transform.eulerAngles.y+360.0f)*(0.5f*Time.deltaTime), Space.World);
                }
                else {
                    flag = 0;
                    tiptext.text = "�����˴��ڿɲٿ�״̬";
                    moving_plate.GetComponent<Moving_plate>().y_rotate_sum = 0.0f;                   
                }
            }
        }
    }
}
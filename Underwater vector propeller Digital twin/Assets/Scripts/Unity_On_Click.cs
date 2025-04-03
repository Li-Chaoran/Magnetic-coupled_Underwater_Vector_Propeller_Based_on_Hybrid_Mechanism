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
    private int flag = 0;     // 若不为0，则正在复位
    public Text tiptext;      // 提示框

    public void Backtozero()
    {
        flag = 1;            // 启动复位
        tiptext.text = "正在复位中，请勿操作";
    } 
    // Start is called before the first frame update
    void Start()
    {
        tiptext.text = "驱动杆处于可操控状态";
    }

    // Update is called once per frame
    void Update()
    {
        // Debug.Log(chuanlianliangan1.GetComponent<joint1_rotation>().dm3410_pose);
        // 复位基座
        if (flag == 1){
            if(chuanlianliangan1.GetComponent<joint1_rotation>().dm3410_pose <= 0.0f){      // 在0-180度之间，使用下面函数复位
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
        // 复位整体末端执行器
        if (flag == 2){
            if(moving_plate_add_axis.transform.eulerAngles.x <= 180.0f){      // 在0-180度之间，使用下面函数复位
                if (Math.Abs(moving_plate_add_axis.transform.eulerAngles.x)>0.05f){
                    moving_plate_add_axis.transform.Rotate(new Vector3(-1.0f, 0.0f, 0.0f), (-moving_plate_add_axis.transform.eulerAngles.x)*(1*Time.deltaTime), Space.World);
                }
                else flag = 3;
                moving_plate_add_axis.GetComponent<All_end_effector>().joint1 = 180.0f+moving_plate_add_axis.transform.eulerAngles.x;
            }
            else if(180.0f<moving_plate_add_axis.transform.eulerAngles.x && moving_plate_add_axis.transform.eulerAngles.x <= 360.0f){
                if (Math.Abs(moving_plate_add_axis.transform.eulerAngles.x-360.0f)>0.05f){  // 扣圈复位
                    moving_plate_add_axis.transform.Rotate(new Vector3(-1.0f, 0.0f, 0.0f), (-moving_plate_add_axis.transform.eulerAngles.x+360.0f)*(1*Time.deltaTime), Space.World);
                }
                else flag = 3;
                moving_plate_add_axis.GetComponent<All_end_effector>().joint1 = 180.0f-moving_plate_add_axis.transform.eulerAngles.x+360.0f;
            }
        }
        // 复位动平台Z轴转向
        if (flag == 3){
            if(moving_plate.transform.eulerAngles.z <= 90.0f){      // 在0-180度之间，使用下面函数复位
                if (Math.Abs(moving_plate.transform.eulerAngles.z-90.0f)>0.05f){
                    moving_plate.transform.Rotate(new Vector3(0.0f, 0.0f, -1.0f), (moving_plate.transform.eulerAngles.z-90.0f)*(0.5f*Time.deltaTime), Space.World);
                }
                else {flag = 4;moving_plate.GetComponent<Moving_plate>().z_rotate_sum = 0.0f;}
            }
            else if(moving_plate.transform.eulerAngles.z > 90.0f){
                if (Math.Abs(moving_plate.transform.eulerAngles.z-90.0f)>0.05f){  // 扣圈复位
                    moving_plate.transform.Rotate(new Vector3(0.0f, 0.0f, 1.0f), (-moving_plate.transform.eulerAngles.z+90.0f)*(0.5f*Time.deltaTime), Space.World);
                }
                else {flag = 4;moving_plate.GetComponent<Moving_plate>().z_rotate_sum = 0.0f;}
            }
        }
        // 复位动平台Y轴转向
        if (flag == 4){
            if(moving_plate.transform.eulerAngles.y <= 180.0f){      // 在0-180度之间，使用下面函数复位
                if (Math.Abs(moving_plate.transform.eulerAngles.y)>0.05f){
                    moving_plate.transform.Rotate(new Vector3(0.0f, -1.0f, 0.0f), moving_plate.transform.eulerAngles.y*(0.5f*Time.deltaTime), Space.World);
                }
                else {
                    flag = 0;
                    tiptext.text = "驱动杆处于可操控状态";
                    moving_plate.GetComponent<Moving_plate>().y_rotate_sum = 0.0f;
                }
            }
            else if(180.0f<moving_plate.transform.eulerAngles.y && moving_plate.transform.eulerAngles.y <= 360.0f){
                if (Math.Abs(moving_plate.transform.eulerAngles.y-360.0f)>0.05f){  // 扣圈复位
                    moving_plate.transform.Rotate(new Vector3(0.0f, 1.0f, 0.0f), (-moving_plate.transform.eulerAngles.y+360.0f)*(0.5f*Time.deltaTime), Space.World);
                }
                else {
                    flag = 0;
                    tiptext.text = "驱动杆处于可操控状态";
                    moving_plate.GetComponent<Moving_plate>().y_rotate_sum = 0.0f;                   
                }
            }
        }
    }
}
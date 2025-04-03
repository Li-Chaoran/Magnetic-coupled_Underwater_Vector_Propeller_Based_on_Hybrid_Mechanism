using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class joint1_rotation : MonoBehaviour
{
    public float dm3410_pose = 0.0f;
    // Start is called before the first frame update
    void Start()
    {
        Debug.Log("游戏开始了");
        transform.SetPositionAndRotation(transform.position, Quaternion.Euler(180.0f, 0.0f, 0.0f));
    }

    // Update is called once per frame
    void Update()
    {
        int z = Input.GetKey(KeyCode.Z) ? 1 : 0;
        int x = Input.GetKey(KeyCode.X) ? 1 : 0;
        if (z != 0)
        {
            dm3410_pose = dm3410_pose + 0.01f; 
            // transform.SetPositionAndRotation(transform.position, Quaternion.Euler(180.0f, -dm3410_pose, 0.0f));
        };
        if (x != 0)
        {
            dm3410_pose = dm3410_pose - 0.01f; 
            // transform.SetPositionAndRotation(transform.position, Quaternion.Euler(180.0f, -dm3410_pose, 0.0f));
        };
        transform.SetPositionAndRotation(transform.position, Quaternion.Euler(180.0f, -dm3410_pose, 0.0f));
        // Debug.Log(joint1);
        // Debug.Log(chuanlianliangan1.GetComponent<joint1_rotation>().dm3410_pose);
        // chuanlianliangan1.GetComponent<joint1_rotation>().dm3410_pose = chuanlianliangan1.GetComponent<joint1_rotation>().dm3410_pose + 10.0f;
    }
}
